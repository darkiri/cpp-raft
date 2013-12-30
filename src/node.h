#ifndef RAFT_NODE
#define RAFT_NODE

#include "log.h"

namespace raft {
  enum NodeState {
    FOLLOWER = 0,
    CANDIDATE = 1,
    LEADER = 2
  };

  struct AppendResult{
    int term;
    bool success;
  };

  // this class is per design not thread safe
  class Node {
    public:
      Node(Log& log): _state(FOLLOWER), _log(log) { };
      NodeState GetState() const {
        return _state;
      };
      const AppendResult AppendEntries(
          int term,
          int prev_log_index,
          int prev_log_term);
    private:
      Node(Node&);
      Node& operator=(const Node&);

      NodeState _state;
      Log& _log;
  };
}
#endif
