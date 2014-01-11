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
    unsigned int leader_commit;
  };

  struct AppendEntriesRes{
    unsigned int term;
    bool success;
  };

  // this class is per design not thread safe
  class Node {
    public:
      Node(Log& log): state_(FOLLOWER), commit_index_(0), log_(log) { };
      NodeState GetState() const {
        return state_;
      };
      unsigned int GetCommitIndex() const {
        return commit_index_;
      };
      const AppendEntriesRes AppendEntries(const AppendEntriesArgs& args);
    private:
      Node(Node&);
      Node& operator=(const Node&);

      NodeState state_;
      unsigned int commit_index_;
      Log& log_;
  };
}
#endif
