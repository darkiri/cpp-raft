#include "node.h"

namespace raft {
  // TODO distinguish lastEntry->term vs last term
  template<class Iter>
  AppendEntriesRes Node<Iter>::AppendEntries(const AppendEntriesArgs& args) {
    auto size = log_.Size();
    auto logIter = log_.Begin();
    auto lastEntry = size > 0 ? logIter[size-1].get() : nullptr;
    auto prevIndexEntry = size > 0 ? logIter[args.prev_log_index].get() : nullptr;
    auto success = IsTermUpToDate(lastEntry, args.term) &&
      (size < 2 || (prevIndexEntry && prevIndexEntry->term == args.prev_log_term));

    if (success) {
      auto index = args.prev_log_index;
      for (auto& e: args.entries) {
        ++index;
        if (index > size - 1) {
          log_.Append(e);
        } else {
          auto ce = logIter[index].get();
          if (ce->term != e.term) {
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
    lastEntry = log_.Size() > 0 ? (--log_.End())->get(): nullptr;
    return CreateRes(lastEntry, success);
  }

  template<class Iter>
  AppendEntriesRes Node<Iter>::RequestVote(const RequestVoteArgs& args) {
    auto size = log_.Size();
    auto logIter = log_.Begin();
    auto lastEntry = size > 0 ? logIter[size-1].get() : nullptr;

    auto success = IsTermUpToDate(lastEntry, args.term) &&
      (voted_for_ == 0 || voted_for_ == args.candidate_id) &&
      IsLogUpToDate(args.last_log_index, args.last_log_term);
    if (success) {
      voted_for_ = args.candidate_id;
    }
    return CreateRes(lastEntry, success);
  }

  template<class Iter>
  bool Node<Iter>::IsTermUpToDate(LogEntry* lastEntry, unsigned int term) const {
    return lastEntry && lastEntry->term <= term;
  }
  template<class Iter>
  bool Node<Iter>::IsLogUpToDate(unsigned int index, unsigned int term) const {
    //TODO check what it meant when log up to date is
    auto entry = (--log_.End())->get();
    return (log_.Size() == index + 1 && entry && entry->term == term);
  }
  template<class Iter>
  AppendEntriesRes Node<Iter>::CreateRes(LogEntry* lastEntry, bool success) const {
    return AppendEntriesRes {
      lastEntry ? lastEntry->term : 0,
      success
    };
  }

  template class Node<InMemoryLog>;
}
