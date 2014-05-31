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

    class TcpTimeoutTests : public ::testing::Test {
      protected:
        TcpTimeoutTests() {}
        virtual ~TcpTimeoutTests() {}

      public:
        void on_timeout();
        condition_variable timeout_;
    };

    void TcpTimeoutTests::on_timeout() {
      LOG_INFO << "Error processing client request.";
      timeout_.notify_one();
    }

    TEST_F(TcpTimeoutTests, ClientFailsFastWithoutServer) {
      config_server conf;
      conf.set_id(1);
      conf.set_port(7574);

      timeout t;
      EXPECT_THROW(rpc::tcp::client(conf, t), boost::system::system_error);
    }

    TEST_F(TcpTimeoutTests, AppendEntries) {
      timeout t;
      config_server conf;
      conf.set_id(1);
      conf.set_port(7574);
      rpc::tcp::server s(conf, t, [](const append_entries_request&) {
          LOG_INFO << "Server waiting for 500 ms.";
          this_thread::sleep_for(chrono::milliseconds(500));
          unique_ptr<append_entries_response> res(new append_entries_response());
          res->set_term(2);
          res->set_success(true);
          return res;
          }, 
          [](const vote_request&){return unique_ptr<vote_response>(new vote_response());},
          [](){});
      s.run();

      rpc::tcp::client c(conf, t);
      unique_ptr<append_entries_request> r(new append_entries_request());
      r->set_term(223);
      r->set_leader_id(2);
      r->set_prev_log_index(222);
      r->set_prev_log_term(333);
      r->set_leader_commit(131);
      c.append_entries_async(move(r), 
          [](const append_entries_response&){},
          bind(&TcpTimeoutTests::on_timeout, this));

      mutex mtx;
      unique_lock<mutex> lck(mtx);
      auto res = timeout_.wait_for(lck, chrono::seconds(5));
      s.stop();
      if (res == cv_status::timeout) {
        FAIL() << "Client haven't produced expected timeout.";
      }
    }

    TEST_F(TcpTimeoutTests, RequestVote) {
      timeout t;
      config_server conf;
      conf.set_id(1);
      conf.set_port(7574);
      rpc::tcp::server s(conf, t, 
          [](const append_entries_request&){return unique_ptr<append_entries_response>(new append_entries_response());},
          [](const vote_request&) {
          LOG_INFO << "Server waiting for 500 ms.";
          this_thread::sleep_for(chrono::milliseconds(500));
          unique_ptr<vote_response> res(new vote_response());
          res->set_term(2);
          res->set_granted(true);
          return res;
          }, 
          [](){});
      s.run();

      rpc::tcp::client c(conf, t);
      unique_ptr<vote_request> r(new vote_request());
      r->set_term(223);
      r->set_candidate_id(2);
      r->set_last_log_term(222);
      r->set_last_log_index(333);
      c.request_vote_async(move(r), 
          [](const vote_response&){},
          bind(&TcpTimeoutTests::on_timeout, this));

      mutex mtx;
      unique_lock<mutex> lck(mtx);
      auto res = timeout_.wait_for(lck, chrono::seconds(5));
      s.stop();
      if (res == cv_status::timeout) {
        FAIL() << "Client haven't produced expected timeout.";
      }
    }
  }
}
