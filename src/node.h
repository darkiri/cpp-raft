#ifndef RAFT_NODE
#define RAFT_NODE

#include "log.h"
#include "proto/raft.pb.h"

namespace raft {
  enum class NodeState {
    FOLLOWER = 0,
    CANDIDATE = 1,
    LEADER = 2
  };

  // this class is per design not thread safe
  template<class TLog>
  class Node {
    public:
      Node(TLog& log):
        state_(NodeState::FOLLOWER),
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
      unsigned int GetCurrentTerm() const {
        return log_.Size() > 0 ? (--log_.End())->term() : 0;
      }
      append_entries_response AppendEntries(const append_entries_request& args);
      vote_response RequestVote(const vote_request& args);

      void StartElection();
    private:
      Node(const Node&);
      Node& operator=(const Node&);

      inline bool IsLogUpToDate(unsigned int index, unsigned int term) const;
      inline void IncrementCurrentTerm(unsigned int newTerm);
      inline void ConvertToFollower();

      NodeState state_;
      unsigned int commit_index_;
      unsigned int voted_for_; // TODO must be persistent
      // TODO currentTerm?
      TLog& log_;
  };
}
#endif
