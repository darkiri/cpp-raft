#include <gtest/gtest.h>
#include "proto/raft.pb.h"
#include "rpc.h"

namespace raft {
  namespace rpc {
    namespace tcp1 {
      class TcpServerTest : public ::testing::Test {
        protected:
          TcpServerTest() {}
          virtual ~TcpServerTest() {}
      };

      append_entries_response test_handler(const append_entries_request&) {
        std::cout<<"append entries handler"<<std::endl;
        return append_entries_response();
      }

      TEST_F(TcpServerTest, Smoke_Test) {
        config_server c;
        c.set_id(1);
        c.set_ip("localhost:7575");
        server s(c, test_handler);
        s.run();
      }

    }
  }
}
