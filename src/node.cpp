#include "node.h"

namespace raft {
  const AppendEntriesRes Node::AppendEntries(const AppendEntriesArgs& args) {
    auto lastEntry = log_.GetLastEntry();
    auto prevIndexEntry = log_.Get(args.prev_log_index);
    auto success = (lastEntry && lastEntry->term < args.term) &&
      (prevIndexEntry && prevIndexEntry->term == args.prev_log_term);
    if (success) {
      auto i = args.prev_log_index;
      auto s = log_.Size();
      for (auto& e: args.entries) {
        i++;
        if (i+1 > s) {
          log_.Append(e);
          continue;
        }
        auto ce = log_.Get(i);
        if (ce->term != e.term) {
          log_.Trim(i);
          s = i - 1;
        }
        log_.Append(e);
      }
    }
    return AppendEntriesRes{
      lastEntry ? lastEntry->term : 0,
      success
    };
  }
}
