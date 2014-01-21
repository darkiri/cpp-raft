#include "node.h"

namespace raft {
  // TODO distinguish lastEntry->term vs last term
  template<class TLog>
  AppendEntriesRes Node<TLog>::AppendEntries(const AppendEntriesArgs& args) {
    auto size = log_.Size();
    auto logIter = log_.Begin();
    auto lastTerm = size == 0 ? 0 : logIter[size-1].term;
    auto prevIndexTerm = size == 0 ? 0 : logIter[args.prev_log_index].term;

    auto success = size != 0 && IsTermUpToDate(lastTerm, args.term) &&
      (size < 2 || prevIndexTerm == args.prev_log_term);

    if (success) {
      auto index = args.prev_log_index;
      for (auto& e: args.entries) {
        ++index;
        if (index > size - 1) {
          log_.Append(e);
        } else {
          auto ce = logIter[index];
          if (ce.term != e.term) {
            log_.Trim(index);
            size = index - 1;
          }
          log_.Append(e);
        }
      }
      commit_index_ = args.leader_commit < log_.Size()
        ? args.leader_commit
        : log_.Size()-1;
    }
    lastTerm = size == 0 ? 0 : logIter[size-1].term;
    return CreateRes(lastTerm, success);
  }

  template<class Iter>
  AppendEntriesRes Node<Iter>::RequestVote(const RequestVoteArgs& args) {
    auto size = log_.Size();
    auto logIter = log_.Begin();
    auto lastTerm = size == 0 ? 0 : logIter[size-1].term;

    auto success = IsTermUpToDate(lastTerm, args.term) &&
      (voted_for_ == 0 || voted_for_ == args.candidate_id) &&
      IsLogUpToDate(args.last_log_index, args.last_log_term);
    if (success) {
      voted_for_ = args.candidate_id;
    }
    return CreateRes(lastTerm, success);
  }

  template<class TNode>
  bool Node<TNode>::IsTermUpToDate(unsigned int lastTerm, unsigned int term) const {
    return lastTerm <= term;
  }

  template<class Iter>
  bool Node<Iter>::IsLogUpToDate(unsigned int index, unsigned int term) const {
    //TODO check what it meant when log up to date is
    auto entry = *(--log_.End());
    return (log_.Size() == index + 1 && entry.term == term);
  }
  template<class TNode>
  AppendEntriesRes Node<TNode>::CreateRes(unsigned int term, bool success) const {
    return AppendEntriesRes {
      term,
      success
    };
  }

  template class Node<InMemoryLog>;
}
