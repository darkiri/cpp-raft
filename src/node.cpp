#include "node.h"

namespace raft {
  // TODO distinguish lastEntry->term vs last term
  AppendEntriesRes Node::AppendEntries(const AppendEntriesArgs& args) {
    auto prevIndexEntry = log_.Get(args.prev_log_index);
    auto s = log_.Size();
    auto success = IsTermUpToDate(args.term) &&
      (s < 2 || (prevIndexEntry && prevIndexEntry->term == args.prev_log_term));
    if (success) {
      auto index = args.prev_log_index;
      for (auto& e: args.entries) {
        ++index;
        if (index >= s) {
          log_.Append(e);
        } else {
          auto ce = log_.Get(index);
          if (ce->term != e.term) {
            log_.Trim(index);
            s = index - 1;
          }
          log_.Append(e);
        }
      }
      commit_index_ = args.leader_commit < log_.Size()
        ? args.leader_commit
        : log_.Size()-1;
    }
    return CreateRes(success);
  }

  AppendEntriesRes Node::RequestVote(const RequestVoteArgs& args) {
    auto success = IsTermUpToDate(args.term) &&
      (voted_for_ == 0 || voted_for_ == args.candidate_id) &&
      IsLogUpToDate(args.last_log_index, args.last_log_term);
    if (success) {
      voted_for_ = args.candidate_id;
    }
    return CreateRes(success);
  }

  bool Node::IsTermUpToDate(unsigned int term) const {
    auto lastEntry = log_.GetLastEntry();
    return lastEntry && lastEntry->term <= term;
  }
  bool Node::IsLogUpToDate(unsigned int index, unsigned int term) const {
    //TODO check what it meand when log up to date is
    auto entry = log_.GetLastEntry();
    return (log_.Size() == index + 1 && entry && entry->term == term);
  }
  AppendEntriesRes Node::CreateRes(bool success) const {
    auto lastEntry = log_.GetLastEntry();
    return AppendEntriesRes {
      lastEntry ? lastEntry->term : 0,
      success
    };
  }
}
