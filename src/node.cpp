#include "node.h"

namespace raft {
  const AppendEntriesRes Node::AppendEntries(const AppendEntriesArgs& args) {
    auto lastEntry = log_.GetLastEntry();
    auto prevIndexEntry = log_.Get(args.prev_log_index);
    auto s = log_.Size();
    auto success = (lastEntry && lastEntry->term <= args.term) &&
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
    }
    return AppendEntriesRes {
      lastEntry ? lastEntry->term : 0,
      success
    };
  }
}
