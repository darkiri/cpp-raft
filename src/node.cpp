#include <algorithm>
#include "node.h"

namespace raft {
  template<class TLog>
  append_entries_response Node<TLog>::AppendEntries(const append_entries_request& args) {
    auto size = log_.Size();
    auto logIter = log_.Begin();
    auto currentTerm = size != 0 ? (logIter + size -1)->term() : 0;
    auto prevIndexTerm = size != 0 ? (logIter + args.prev_log_index())->term() : 0;

    auto success = size != 0 && args.term() >= currentTerm &&
      (size <= 1 || prevIndexTerm == args.prev_log_term());

    if (success && args.term() > currentTerm){
      ConvertToFollower();
    }

    if (success && args.entries().begin() != args.entries().end()) {
      if (args.prev_log_index() == size - 1) {
        std::for_each(
            args.entries().begin(),
            args.entries().end(),
            [this](const log_entry& e) { log_.Append(e);});
      } else {
        auto nodeIter = logIter + args.prev_log_index() + 1;
        auto masterIter = args.entries().begin();
        while(nodeIter->term() == masterIter->term()) {
          ++nodeIter;
          ++masterIter;
        }
        log_.Trim(nodeIter);
        std::for_each(
            masterIter,
            args.entries().end(),
            [this](const log_entry& e) { log_.Append(e);});
      }
    } else if (success && args.term() > currentTerm) {
      IncrementCurrentTerm(args.term());
    }

    if (success) {
      commit_index_ = args.leader_commit() < log_.Size()
        ? args.leader_commit()
        : log_.Size()-1;
    }
    currentTerm = size != 0 ? logIter[size-1].term() : 0;
    append_entries_response response;
    response.set_term(currentTerm);
    response.set_success(success);
    return response;
  }

  template<class Iter>
  vote_response Node<Iter>::RequestVote(const vote_request& args) {
    auto size = log_.Size();
    auto logIter = log_.Begin();
    auto currentTerm = size != 0 ? (logIter + size -1)->term() : 0;

    auto success = args.term() >= currentTerm &&
      (voted_for_ == 0 || voted_for_ == args.candidate_id()) &&
      IsLogUpToDate(args.last_log_index(), args.last_log_term());
    if (success) {
      voted_for_ = args.candidate_id();
    }
    if (args.term() > currentTerm) {
      ConvertToFollower();
      IncrementCurrentTerm(args.term());
    }
    vote_response response;
    response.set_term(currentTerm);
    response.set_granted(success);
    return response;
  }

  template<class Iter>
  bool Node<Iter>::IsLogUpToDate(unsigned int index, unsigned int term) const {
    //TODO check what it meant when log up to date is
    auto entry = *(--log_.End());
    return (log_.Size() == index + 1 && entry.term() == term);
  }


  template<class Iter>
  void Node<Iter>::StartElection(){
    state_ = NodeState::CANDIDATE;
    IncrementCurrentTerm(GetCurrentTerm()+1);
  }

  template<class Iter>
  void Node<Iter>::IncrementCurrentTerm(unsigned int newTerm){
    log_entry e;
    e.set_term(newTerm);
    log_.Append(e);
  }

  template<class Iter>
  void Node<Iter>::ConvertToFollower(){
    state_ = NodeState::FOLLOWER;
  }
  template class Node<InMemoryLog>;
}
