#include <algorithm>
#include "node.h"
#include "logging.h"

namespace raft {
  template<class TLog, class TStateMachine>
  bool node<TLog, TStateMachine>::log_matching(const append_entries_request& request) const {
    auto size = plog_->size();
    auto logIter = plog_->begin();
    
    auto prevIndexTerm = size < request.prev_log_index()
        ? -1
        : (logIter + request.prev_log_index())->term();

    return prevIndexTerm == request.prev_log_term();
  }

  template<class TLog, class TStateMachine>
  bool node<TLog, TStateMachine>::is_log_uptodate(unsigned int index, unsigned int term) const {
    //TODO replace with log_matching
    auto entry = *(--plog_->end());
    return (plog_->size() == index + 1 && entry.term() == term);
  }


  template<class TLog, class TStateMachine>
  append_entries_response node<TLog, TStateMachine>::append_entries(const append_entries_request& args) {

    this->ensure_current_term(args.term());

    auto remote_term_outdated = args.term() < plog_->current_term();
    const auto success = !remote_term_outdated && this->log_matching(args);

    if (success) {
      this->do_append_entries(args);

      if (args.leader_commit() > commit_index_) {
        commit_index_ = std::min((unsigned int)args.leader_commit(), plog_->size()-1);
      }
    }

    append_entries_response response;
    response.set_term(plog_->current_term());
    response.set_success(success);
    return response;
  }

  template<class TLog, class TStateMachine>
  void node<TLog, TStateMachine>::do_append_entries(const append_entries_request& args) {
    if (args.entries().begin() != args.entries().end()) {
      if (args.prev_log_index() == plog_->size() - 1) {
        std::for_each(
            args.entries().begin(),
            args.entries().end(),
            [this](const log_entry& e) { plog_->append(e);});
      } else {
        auto nodeIter = plog_->begin() + args.prev_log_index() + 1;
        auto masterIter = args.entries().begin();
        while(nodeIter->term() == masterIter->term()) {
          ++nodeIter;
          ++masterIter;
        }
        plog_->trim(nodeIter);
        std::for_each(
            masterIter,
            args.entries().end(),
            [this](const log_entry& e) { plog_->append(e);});
      }
    }
  }

  template<class TLog, class TStateMachine>
  vote_response node<TLog, TStateMachine>::request_vote(const vote_request& args) {

    this->ensure_current_term(args.term());

    auto currentTerm = plog_->current_term();

    auto granted = args.term() >= currentTerm &&
      (plog_->voted_for() == 0 || plog_->voted_for() == args.candidate_id()) &&
      is_log_uptodate(args.last_log_index(), args.last_log_term());

    if (granted) {
      plog_->set_voted_for(args.candidate_id());
    }

    vote_response response;
    response.set_term(currentTerm);
    response.set_granted(granted);
    return response;
  }

  template<class TLog, class TStateMachine>
  void node<TLog, TStateMachine>::ensure_current_term(unsigned int term) {
    if (term > plog_->current_term()){
      plog_->set_current_term(term);
      this->convert_to_follower();
    }
  }

  template<class TLog, class TStateMachine>
  void node<TLog, TStateMachine>::start_election(){
    state_ = node_state::CANDIDATE;
    plog_->set_current_term(plog_->current_term() + 1);
  }

  template<class TLog, class TStateMachine>
  void node<TLog, TStateMachine>::convert_to_follower(){
    state_ = node_state::FOLLOWER;
  }

  template class node<in_memory_log, state_machine>;
}
