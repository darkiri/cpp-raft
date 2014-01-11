#ifndef RAFT_NODE
#define RAFT_NODE

#include "log.h"

namespace raft {
  enum NodeState {
    FOLLOWER = 0,
    CANDIDATE = 1,
    LEADER = 2
  };

  struct AppendEntriesArgs {
    unsigned int term;
    unsigned int prev_log_index;
    unsigned int prev_log_term;
    std::vector<LogEntry> entries;
    unsigned int leaderCommit;
  };

  struct AppendEntriesRes{
    unsigned int term;
    bool success;
  };

  // this class is per design not thread safe
  class Node {
    public:
      Node(Log& log): _state(FOLLOWER), _log(log) { };
      NodeState GetState() const {
        return _state;
      };
      const AppendEntriesRes AppendEntries(const AppendEntriesArgs& args);
    private:
      Node(Node&);
      Node& operator=(const Node&);

      NodeState _state;
      Log& _log;
  };
}
#endif
