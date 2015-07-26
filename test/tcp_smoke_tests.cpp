#include <gtest/gtest.h>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>
#include "rpc.h"
#include "logging.h"

namespace raft {
  namespace rpc {
    using namespace std;
    using namespace std::placeholders;

    class TcpSmokeTests : public ::testing::Test {
      protected:
        TcpSmokeTests() : message_received_(raft_message::NONE), append_response_(), vote_response_() {}
        virtual ~TcpSmokeTests() {}

      public:
        void on_appended(const append_entries_response& r);
        void on_voted(const vote_response& r);
        raft_message_type message_received_;
        append_entries_response append_response_;
        vote_response vote_response_;
        condition_variable processed_;
        condition_variable processed_2nd_;
    };

    unique_ptr<append_entries_response> append_test_handler(const append_entries_request& r) {
      LOG_INFO << "Append entries handler";
      LOG_INFO << "Request: term=" << r.term();
      unique_ptr<append_entries_response> res(new append_entries_response());
      res->set_term(2);
      res->set_success(true);
      return res;
    }

    unique_ptr<vote_response> vote_test_handler(const vote_request& r) {
      LOG_INFO << "Request vote handler";
      LOG_INFO << "Request: term=" << r.term();
      unique_ptr<vote_response> res(new vote_response());
      res->set_term(5);
      res->set_granted(true);
      return res;
    }

    void TcpSmokeTests::on_appended(const append_entries_response& r) {
      LOG_INFO << "Append Response: term = " << r.term() << ", success = " << r.success();
      message_received_ = raft_message::APPEND_ENTRIES;
      append_response_ = r;
      processed_.notify_one();
    }

    void TcpSmokeTests::on_voted(const vote_response& r) {
      LOG_INFO << "Vote Response: term = " << r.term() << ", granted = " << r.granted();
      message_received_ = raft_message::VOTE;
      vote_response_ = r;
      processed_.notify_one();
    }

    TEST_F(TcpSmokeTests, AppendEntries) {
      config_server conf;
      conf.set_id(1);
      conf.set_port(7574);

      rpc::tcp::server s(conf, append_test_handler, vote_test_handler);
      s.run();

      rpc::tcp::client c(conf, bind(&TcpSmokeTests::on_appended, this, _1), [](vote_response){});
      unique_ptr<append_entries_request> r(new append_entries_request());
      r->set_term(223);
      r->set_leader_id(2);
      r->set_prev_log_index(222);
      r->set_prev_log_term(333);
      r->set_leader_commit(131);
      c.append_entries_async(move(r));

      mutex mtx;
      unique_lock<mutex> lck(mtx);
      processed_.wait_for(lck, std::chrono::seconds(1));

      EXPECT_EQ(raft_message::APPEND_ENTRIES, message_received_);
      EXPECT_EQ(append_response_.term(), 2);
      EXPECT_TRUE(append_response_.success());
      s.stop();
    }

    TEST_F(TcpSmokeTests, RequestVote) {
      config_server conf;
      conf.set_id(1);
      conf.set_port(7574);
      rpc::tcp::server s(conf, append_test_handler, vote_test_handler);
      s.run();

      rpc::tcp::client c(conf, [](append_entries_response){}, bind(&TcpSmokeTests::on_voted, this, _1));
      unique_ptr<vote_request> r(new vote_request());
      r->set_term(223);
      r->set_candidate_id(10);
      r->set_last_log_term(333);
      r->set_last_log_index(222);
      c.request_vote_async(move(r));

      mutex mtx;
      unique_lock<mutex> lck(mtx);
      processed_.wait_for(lck, chrono::seconds(1));

      EXPECT_EQ(raft_message::VOTE, message_received_);
      EXPECT_EQ(vote_response_.term(), 5);
      EXPECT_TRUE(vote_response_.granted());
      s.stop();
    }

    TEST_F(TcpSmokeTests, AppendEntries_AppendEntries) {
      config_server conf;
      conf.set_id(1);
      conf.set_port(7574);

      rpc::tcp::server s(conf, append_test_handler, vote_test_handler);
      s.run();

      rpc::tcp::client c(conf, bind(&TcpSmokeTests::on_appended, this, _1), [](vote_response){});

      unique_ptr<append_entries_request> r(new append_entries_request());
      r->set_term(223);
      r->set_leader_id(2);
      r->set_prev_log_index(222);
      r->set_prev_log_term(333);
      r->set_leader_commit(131);
      c.append_entries_async(move(r));

      unique_ptr<append_entries_request> r1(new append_entries_request());
      r1->set_term(224);
      r1->set_leader_id(2);
      r1->set_prev_log_index(222);
      r1->set_prev_log_term(333);
      r1->set_leader_commit(131);
      c.append_entries_async(move(r1));

      mutex mtx;
      unique_lock<mutex> lck(mtx);
      processed_.wait_for(lck, std::chrono::seconds(1));

      EXPECT_EQ(append_response_.term(), 2);
      EXPECT_TRUE(append_response_.success());
      s.stop();
    }

    TEST_F(TcpSmokeTests, AppendEntries_RequestVote) {
      config_server conf;
      conf.set_id(1);
      conf.set_port(7574);

      rpc::tcp::server s(conf, append_test_handler, vote_test_handler);
      s.run();

      rpc::tcp::client c(conf, 
          bind(&TcpSmokeTests::on_appended, this, _1),
          bind(&TcpSmokeTests::on_voted, this, _1));
      unique_ptr<append_entries_request> r(new append_entries_request());
      r->set_term(223);
      r->set_leader_id(2);
      r->set_prev_log_index(222);
      r->set_prev_log_term(333);
      r->set_leader_commit(131);
      r->add_entries()->set_term(224);

      c.append_entries_async(move(r));

      unique_ptr<vote_request> r2(new vote_request());
      r2->set_term(223);
      r2->set_candidate_id(10);
      r2->set_last_log_term(333);
      r2->set_last_log_index(222);
      c.request_vote_async(move(r2));

      mutex mtx;
      unique_lock<mutex> lck(mtx);
      processed_.wait_for(lck, std::chrono::seconds(1));
      processed_2nd_.wait_for(lck, std::chrono::seconds(1));

      EXPECT_EQ(append_response_.term(), 2);
      EXPECT_TRUE(append_response_.success());
      EXPECT_EQ(vote_response_.term(), 5);
      EXPECT_TRUE(vote_response_.granted());
      s.stop();
    }
  }
}
