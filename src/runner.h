#ifndef RAFT_RUNNER
#define RAFT_RUNNER

#include <memory>
#include "proto/raft.pb.h"
#include "timeout.h"
#include "node.h"


namespace raft {
  template<class TLog, class TRpcClient, class TRpcServer>
  class runner {
    public:
      runner(const config&, timeout&);
      ~runner();

      runner(const runner&) = delete;
      runner& operator=(const runner&) = delete;

      void run();
      void stop();
    private:
      node<TLog> node_;
  };
}
#endif
