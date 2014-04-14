#include <gtest/gtest.h>
#include "proto/raft.pb.h"
#include "rpc.h"

namespace raft {
  namespace rpc {
    class TcpServerTest : public ::testing::Test {
      protected:
        TcpServerTest() {}
        virtual ~TcpServerTest() {}
    };

    append_entries_response test_handler(const append_entries_request&) {
      std::cout<<"append entries handler"<<std::endl;
      return append_entries_response();
    }
  }
}
