#include <gtest/gtest.h>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>
#include "proto/raft.pb.h"
#include "rpc.h"
#include <boost/asio.hpp>
#include "logging.h"

namespace raft {
  namespace rpc {
    using namespace std;

    class TcpServerTest : public ::testing::Test {
      protected:
        TcpServerTest() : response_() {}
        virtual ~TcpServerTest() {}

        void call_append_entries(rpc::tcp::client& c, const append_entries_request& r);
        void call_request_vote(rpc::tcp::client& c, const vote_request& r);
        void on_appended(const append_entries_response& r);
        void on_voted(const vote_response& r);
        void on_timeout();
        append_entries_response response_;
        vote_response vote_response_;
        condition_variable entriesAppended_;
        condition_variable voted_;
        condition_variable timeout_;
    };

    append_entries_response test_handler(const append_entries_request& r) {
      LOG_INFO << "append entries handler";
      LOG_INFO << "request: term=" << r.term();
      append_entries_response res;
      res.set_term(2);
      res.set_success(true);
      return res;
    }

    vote_response vote_test_handler(const vote_request& r) {
      LOG_INFO << "request vote handler";
      LOG_INFO << "request: term=" << r.term();
      vote_response res;
      res.set_term(5);
      res.set_granted(true);
      return res;
    }

    void TcpServerTest::on_appended(const append_entries_response& r) {
      LOG_INFO << "Append Response: term = " << r.term() << ", success = " << r.success();
      response_ = r;
      entriesAppended_.notify_one();
    }

    void TcpServerTest::on_voted(const vote_response& r) {
      LOG_INFO << "Vote Response: term = " << r.term() << ", granted = " << r.granted();
      vote_response_ = r;
      voted_.notify_one();
    }

    void TcpServerTest::on_timeout() {
      LOG_INFO << " timeout in append entries ";
      timeout_.notify_one();
    }

    void TcpServerTest::call_append_entries(rpc::tcp::client& c, const append_entries_request& r) {
      c.append_entries_async(r, 
          [this](append_entries_response r) {on_appended(r);},
          [this]() {on_timeout();});
    }

    void TcpServerTest::call_request_vote(rpc::tcp::client& c, const vote_request& r) {
      c.request_vote_async(r, 
          [this](vote_response r) {on_voted(r);},
          [this]() {on_timeout();});
    }

    TEST_F(TcpServerTest, SmokeTest_AppendEntries) {
      timeout t;
      config_server conf;
      conf.set_id(1);
      conf.set_port(7574);
      rpc::tcp::server s(conf, t, test_handler, vote_test_handler, [this](){on_timeout();});
      s.run();

      rpc::tcp::client c(conf, t);
      append_entries_request r;
      r.set_term(223);
      call_append_entries(c, r);

      mutex mtx;
      unique_lock<mutex> lck(mtx);
      entriesAppended_.wait_for(lck, std::chrono::seconds(1));

      EXPECT_EQ(response_.term(), 2);
      EXPECT_TRUE(response_.success());
      s.stop();
    }

    TEST_F(TcpServerTest, SmokeTest_RequestVote) {
      timeout t;
      config_server conf;
      conf.set_id(1);
      conf.set_port(7574);
      rpc::tcp::server s(conf, t, test_handler, vote_test_handler,
          [this](){on_timeout();});
      s.run();

      rpc::tcp::client c(conf, t);
      vote_request r;
      r.set_term(223);
      r.set_candidate_id(10);
      call_request_vote(c, r);

      mutex mtx;
      unique_lock<mutex> lck(mtx);
      voted_.wait_for(lck, chrono::seconds(5));

      EXPECT_EQ(vote_response_.term(), 5);
      EXPECT_TRUE(vote_response_.granted());
      s.stop();
    }

    TEST_F(TcpServerTest, ClientFailsFastWithoutServer) {
      config_server conf;
      conf.set_id(1);
      conf.set_port(7574);

      timeout t;
      EXPECT_THROW(rpc::tcp::client(conf, t), boost::system::system_error);
    }

    TEST_F(TcpServerTest, ClientTimeoutTest_AppendEntries) {
      timeout t;
      config_server conf;
      conf.set_id(1);
      conf.set_port(7574);
      rpc::tcp::server s(conf, t, [](const append_entries_request&) {
          append_entries_response res;
          LOG_INFO << " Server waiting for 500 ms";
          this_thread::sleep_for(chrono::milliseconds(500));
          res.set_term(2);
          res.set_success(true);
          return res;
          }, 
          [](const vote_request&){return vote_response();},
          [](){});
      s.run();

      rpc::tcp::client c(conf, t);
      append_entries_request r;
      r.set_term(223);
      call_append_entries(c, r);

      mutex mtx;
      unique_lock<mutex> lck(mtx);
      auto res = timeout_.wait_for(lck, chrono::seconds(5));
      LOG_INFO << "Stopping server";
      s.stop();
      if (res == cv_status::timeout) {
        FAIL() << "Client haven't produced expected timeout.";
      }
    }
  }
}
