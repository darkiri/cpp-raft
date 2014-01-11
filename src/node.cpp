#include "node.h"

namespace raft {
  const AppendEntriesRes Node::AppendEntries(const AppendEntriesArgs& args) {
    auto lastEntry = _log.GetLastEntry();
    auto prevIndexEntry = _log.Get(args.prev_log_index);
    auto success = (lastEntry && lastEntry->term < args.term) &&
      (prevIndexEntry && prevIndexEntry->term == args.prev_log_term);
    if (success) {
      auto i = args.prev_log_index;
      auto s = _log.Size();
      for (auto& e: args.entries) {
        i++;
        if (i+1 > s) {
          _log.Append(e);
          continue;
        }
        auto ce = _log.Get(i);
        if (ce->term != e.term) {
          _log.Trim(i);
          s = i - 1;
        }
        _log.Append(e);
      }
    }
    return AppendEntriesRes{
      lastEntry ? lastEntry->term : 0,
      success
    };
  }
}
