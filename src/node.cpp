#include "node.h"

namespace raft {
  const AppendResult Node::AppendEntries(
      unsigned int term,
      unsigned int prev_log_index,
      unsigned int prev_log_term,
      std::vector<LogEntry> entries,
      unsigned int /*leaderCommit*/) {
    auto lastEntry = _log.GetLastEntry();
    auto prevIndexEntry = _log.Get(prev_log_index);
    auto success = (lastEntry && lastEntry->term < term) &&
      (prevIndexEntry && prevIndexEntry->term == prev_log_term);
    if (success) {
      auto i = prev_log_index;
      auto s = _log.Size();
      for (auto& e: entries) {
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
    return AppendResult{
      lastEntry ? lastEntry->term : 0,
      success
    };
  }
}
