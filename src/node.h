#ifndef RAFT_NODE
#define RAFT_NODE

#include "log.h"
#include "proto/raft.pb.h"

namespace raft {
  enum class node_state {
    FOLLOWER = 0,
    CANDIDATE = 1,
    LEADER = 2
  };

  // this class is per design not thread safe
  template<class TLog>
  class node {
    public:
      node(TLog& log):
        state_(node_state::FOLLOWER),
        commit_index_(0),
        voted_for_(0),
        log_(log) { };

      node_state state() const {
        return state_;
      };
      unsigned int commit_index() const {
        return commit_index_;
      };
      unsigned int voted_for() const {
        return voted_for_;
      }
      unsigned int current_term() const {
        return log_.size() > 0 ? (--log_.end())->term() : 0;
      }
      append_entries_response append_entries(const append_entries_request& args);
      vote_response request_vote(const vote_request& args);

      void start_election();
    private:
      node(const node&);
      node& operator=(const node&);

      inline bool is_log_uptodate(unsigned int index, unsigned int term) const;
      inline void increment_current_term(unsigned int newTerm);
      inline void convert_to_follower();

      node_state state_;
      unsigned int commit_index_;
      unsigned int voted_for_; // TODO must be persistent
      // TODO currentTerm?
      TLog& log_;
  };
}
#endif
