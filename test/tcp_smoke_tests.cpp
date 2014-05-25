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
        void on_appended(const append_entries_response& r);
        void on_timeout();
        append_entries_response response_;
        condition_variable entriesAppended_;
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

    void TcpServerTest::on_appended(const append_entries_response& r) {
      LOG_INFO << "Response: term = " << r.term() << ", success = " << r.success();
      response_ = r;
      entriesAppended_.notify_one();
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

    TEST_F(TcpServerTest, SmokeTest) {
      config_server conf;
      conf.set_id(1);
      conf.set_port(7574);
      rpc::tcp::server s(conf, test_handler);
      s.run();

      timeout t;
      rpc::tcp::client c(conf, t);
      append_entries_request r;
      r.set_term(223);
      call_append_entries(c, r);

      mutex mtx;
      unique_lock<mutex> lck(mtx);
      entriesAppended_.wait(lck);

      EXPECT_EQ(response_.term(), 2);
      EXPECT_TRUE(response_.success());
      s.stop();
    }

    TEST_F(TcpServerTest, ClientFailsFastWithoutServer) {
      config_server conf;
      conf.set_id(1);
      conf.set_port(7574);

      timeout t;
      EXPECT_THROW(rpc::tcp::client(conf, t), boost::system::system_error);
    }

    TEST_F(TcpServerTest, ClientTimeoutTest_AppenEntries) {
      config_server conf;
      conf.set_id(1);
      conf.set_port(7574);
      rpc::tcp::server s(conf, [](const append_entries_request&) {
          append_entries_response res;
          LOG_INFO << " wait for 1 sec";
          this_thread::sleep_for(chrono::seconds(1));
          LOG_INFO << " after 1 sec";
          res.set_term(2);
          res.set_success(true);
          return res;
          });
      s.run();

      timeout t;
      rpc::tcp::client c(conf, t);
      append_entries_request r;
      r.set_term(223);
      call_append_entries(c, r);

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
