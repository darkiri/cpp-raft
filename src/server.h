#ifndef RAFT_SERVER
#define RAFT_SERVER

#include "node.h"
#include "rpc.h"
#include "timeout.h"
#include "config.h"

namespace raft {
  template<class RpcServer, class RpcClient, class node>
  class server {
    public:
      server(const server&) = delete;
      server(server&&) = delete;

      void run();
      void stop();
    private:
      const config& peers_;
      node node_;
      RpcServer rpc_server_;
      RpcClient rpc_client_;
  }
}
#endif
