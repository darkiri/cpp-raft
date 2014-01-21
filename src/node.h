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

  struct RequestVoteArgs {
    unsigned int term;
    unsigned int candidate_id;
    unsigned int last_log_index;
    unsigned int last_log_term;
  };

  // this class is per design not thread safe
  template<class TLog>
  class Node {
    public:
      Node(TLog& log):
        state_(FOLLOWER),
        commit_index_(0),
        voted_for_(0),
        log_(log) { };

      NodeState GetState() const {
        return state_;
      };
      unsigned int GetCommitIndex() const {
        return commit_index_;
      };
      unsigned int GetVotedFor() const {
        return voted_for_;
      }
      AppendEntriesRes AppendEntries(const AppendEntriesArgs& args);
      AppendEntriesRes RequestVote(const RequestVoteArgs& args);
    private:
      Node(const Node&);
      Node& operator=(const Node&);

      inline AppendEntriesRes CreateRes(LogEntry* lastEntry, bool success) const;
      inline bool IsTermUpToDate(LogEntry* lastEntry, unsigned int term) const;
      inline bool IsLogUpToDate(unsigned int index, unsigned int term) const;

      NodeState state_;
      unsigned int commit_index_;
      unsigned int voted_for_; // TODO must be persistent
      // TODO currentTerm?
      TLog& log_;
  };
}
#endif
