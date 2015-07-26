#ifndef RAFT_NODE
#define RAFT_NODE

#include "log.h"
#include "state_machine.h"
#include "proto/raft.pb.h"

namespace raft {
  enum class node_state {
    FOLLOWER = 0,
    CANDIDATE = 1,
    LEADER = 2
  };

  // this class is per design not thread safe
  template<class TLog, class TStateMachine>
  class node {
    public:
      node(std::shared_ptr<TLog> plog, std::shared_ptr<TStateMachine> pstate_machine):
        state_(node_state::FOLLOWER),
        commit_index_(0),
        last_applied_(0),
        plog_(plog),
        pstate_machine_(pstate_machine) { };

      node_state state() const {
        return state_;
      };

      unsigned int commit_index() const {
        return commit_index_;
      };

      unsigned int last_applied() const {
        return last_applied_;
      };

      append_entries_response append_entries(const append_entries_request& args);
      vote_response request_vote(const vote_request& args);

      void start_election();
    private:
      node(const node&);
      node& operator=(const node&);

      inline bool is_log_uptodate(unsigned int index, unsigned int term) const;
      inline bool log_matching(const append_entries_request& request) const;
      inline void convert_to_follower();
      inline void do_append_entries(const append_entries_request& args);
      inline void ensure_current_term(unsigned int term);

      node_state state_;
      unsigned int commit_index_;
      unsigned int last_applied_;
      std::shared_ptr<TLog> plog_;
      std::shared_ptr<TStateMachine> pstate_machine_;
  };
}
#endif
