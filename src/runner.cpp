#include "runner.h"
#include "logging.h"

namespace raft {

  template<class TLog, class TRpcClient, class TRpcServer>
  runner<TLog, TRpcClient, TRpcServer>::runner(const config&, timeout&) {
    // initialize node from config
    // from the election timeout define the heartbeat interval
  }

  template<class TLog, class TRpcClient, class TRpcServer>
  void runner<TLog, TRpcClient, TRpcServer>::run() {
    // start election
    // manage the node state accortind to the raft paper
    // send requests to the remotes, handle responses and timeouts
  }

  template<class TLog, class TRpcClient, class TRpcServer>
  void runner<TLog, TRpcClient, TRpcServer>::stop() {
  }
}
