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
      runner(const config& c, timeout& t) : pimpl_(c, t) {};

      runner(const runner&) = delete;
      runner& operator=(const runner&) = delete;

      void run();
      void stop();
    private:
      struct impl;
      std::unique_ptr<impl> pimpl_;
  };
}
#endif
