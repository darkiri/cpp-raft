#include <algorithm>
#include "node.h"

namespace raft {
  template<class TLog>
  AppendEntriesRes Node<TLog>::AppendEntries(const AppendEntriesArgs& args) {
    auto size = log_.Size();
    auto logIter = log_.Begin();
    auto lastTerm = size == 0 ? 0 : (logIter + size -1)->term;
    auto prevIndexTerm = size == 0 ? 0 : (logIter + args.prev_log_index)->term;

    auto success = size != 0 && lastTerm <= args.term &&
      (size < 2 || prevIndexTerm == args.prev_log_term);

    if (success && args.entries.begin() != args.entries.end()) {
      if (args.prev_log_index == size - 1) {
        log_.Append(args.entries.begin(), args.entries.end());
      } else {
        auto nodeIter = logIter + args.prev_log_index + 1;
        auto masterIter = args.entries.begin();
        while(nodeIter->term == masterIter->term) {
          ++nodeIter;
          ++masterIter;
        }
        log_.Trim(nodeIter);
        log_.Append(masterIter, args.entries.end());
      }
    }
    if (success) {
      commit_index_ = args.leader_commit < log_.Size()
        ? args.leader_commit
        : log_.Size()-1;
    }
    lastTerm = size == 0 ? 0 : logIter[size-1].term;
    return AppendEntriesRes {
      lastTerm,
      success
    };
  }

  template<class Iter>
  AppendEntriesRes Node<Iter>::RequestVote(const RequestVoteArgs& args) {
    auto size = log_.Size();
    auto logIter = log_.Begin();
    auto lastTerm = size == 0 ? 0 : logIter[size-1].term;

    auto success = lastTerm <= args.term &&
      (voted_for_ == 0 || voted_for_ == args.candidate_id) &&
      IsLogUpToDate(args.last_log_index, args.last_log_term);
    if (success) {
      voted_for_ = args.candidate_id;
    }
    return AppendEntriesRes {
      lastTerm,
      success
    };
  }

  template<class Iter>
  bool Node<Iter>::IsLogUpToDate(unsigned int index, unsigned int term) const {
    //TODO check what it meant when log up to date is
    auto entry = *(--log_.End());
    return (log_.Size() == index + 1 && entry.term == term);
  }

  template class Node<InMemoryLog>;
}
