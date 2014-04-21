#include <gtest/gtest.h>
#include <mutex>
#include <condition_variable>
#include "proto/raft.pb.h"
#include "rpc.h"

namespace raft {
  namespace rpc {
    using namespace std;

    class TcpServerTest : public ::testing::Test {
      protected:
        TcpServerTest() : response_() {}
        virtual ~TcpServerTest() {}

        void on_appended(const append_entries_response& r);
        append_entries_response response_;
        condition_variable entriesAppended_;
    };

    append_entries_response test_handler(const append_entries_request& r) {
      cout << "append entries handler" << endl;
      cout << "request: term=" << r.term() << endl;
      append_entries_response res;
      res.set_term(2);
      res.set_success(true);
      return res;
    }

    void TcpServerTest::on_appended(const append_entries_response& r) {
      cout << "Response: term = " << r.term() << ", success = " << r.success() << endl;
      response_ = r;
      entriesAppended_.notify_one();
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
      c.append_entries_async(r, [this](append_entries_response r) {on_appended(r);});

      mutex mtx;
      unique_lock<mutex> lck(mtx);
      entriesAppended_.wait(lck);

      EXPECT_EQ(response_.term(), 2);
      EXPECT_TRUE(response_.success());
      s.stop();
    }
  }
}
