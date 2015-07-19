#include <algorithm>
#include "node.h"
#include "logging.h"

namespace raft {
  template<class TLog>
  bool node<TLog>::log_matching(const append_entries_request& request) const {
    auto size = log_.size();
    auto logIter = log_.begin();
    
    auto prevIndexTerm = size < request.prev_log_index()
        ? -1
        : (logIter + request.prev_log_index())->term();

    return prevIndexTerm == request.prev_log_term();
  }

  template<class TLog>
  append_entries_response node<TLog>::append_entries(const append_entries_request& args) {

    auto remote_term_outdated = args.term() < log_.current_term();
    const auto success = !remote_term_outdated && log_matching(args);

    auto current_term_outdated = log_.current_term() < args.term();
    if (current_term_outdated){
      convert_to_follower();
    }

    if (success && args.entries().begin() != args.entries().end()) {
      if (args.prev_log_index() == log_.size() - 1) {
        std::for_each(
            args.entries().begin(),
            args.entries().end(),
            [this](const log_entry& e) { log_.append(e);});
      } else {
        auto nodeIter = log_.begin() + args.prev_log_index() + 1;
        auto masterIter = args.entries().begin();
        while(nodeIter->term() == masterIter->term()) {
          ++nodeIter;
          ++masterIter;
        }
        log_.trim(nodeIter);
        std::for_each(
            masterIter,
            args.entries().end(),
            [this](const log_entry& e) { log_.append(e);});
      }
    } else if (success && args.term() > log_.current_term()) {
      log_.set_current_term(args.term());
    }

    if (success) {
      commit_index_ = args.leader_commit() < log_.size()
        ? args.leader_commit()
        : log_.size()-1;
    }
    append_entries_response response;
    response.set_term(log_.current_term());
    response.set_success(success);
    return response;
  }

  template<class Iter>
  vote_response node<Iter>::request_vote(const vote_request& args) {
    auto currentTerm = log_.current_term();

    auto success = args.term() >= currentTerm &&
      (log_.voted_for() == 0 || log_.voted_for() == args.candidate_id()) &&
      is_log_uptodate(args.last_log_index(), args.last_log_term());
    if (success) {
      log_.set_voted_for(args.candidate_id());
    }
    if (args.term() > currentTerm) {
      convert_to_follower();
      log_.set_current_term(args.term());
    }
    vote_response response;
    response.set_term(currentTerm);
    response.set_granted(success);
    return response;
  }

  template<class Iter>
  bool node<Iter>::is_log_uptodate(unsigned int index, unsigned int term) const {
    //TODO check what it meant when log up to date is
    auto entry = *(--log_.end());
    return (log_.size() == index + 1 && entry.term() == term);
  }


  template<class Iter>
  void node<Iter>::start_election(){
    state_ = node_state::CANDIDATE;
    log_.set_current_term(log_.current_term() + 1);
  }

  template<class Iter>
  void node<Iter>::convert_to_follower(){
    state_ = node_state::FOLLOWER;
  }
  template class node<in_memory_log>;
}
