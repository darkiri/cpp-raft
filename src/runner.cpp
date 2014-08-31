#include "runner.h"
#include "logging.h"

namespace raft {

  template<class TLog, class TRpcClient, class TRpcServer>
  struct runner<TLog, TRpcClient, TRpcServer>::impl {
    public:
      impl(const config& c, timeout& t):
        config_(c),
        timeout_(t),
        heartbeat_in_(t.get()/2) {};
      ~impl();

      void run();
      void stop();
    private:
      const config& config_;
      timeout& timeout_;
      const int heartbeat_in_;
  };

  template<class TLog, class TRpcClient, class TRpcServer>
  void runner<TLog, TRpcClient, TRpcServer>::impl::run() {
    // initialize node from config
    // start election
    // manage the node state accortind to the raft paper
    // send requests to the remotes, handle responses and timeouts
  }

  template<class TLog, class TRpcClient, class TRpcServer>
  void runner<TLog, TRpcClient, TRpcServer>::impl::stop() {
  }

  template<class TLog, class TRpcClient, class TRpcServer>
  void runner<TLog, TRpcClient, TRpcServer>::run() {
    pimpl_->run();
  }

  template<class TLog, class TRpcClient, class TRpcServer>
  void runner<TLog, TRpcClient, TRpcServer>::stop() {
    pimpl_->stop();
  }
}
