#include "node.h"

namespace raft {
  const AppendResult Node::AppendEntries(int term, int prev_log_index, int prev_log_term) {
    auto lastEntry = _log.GetLastEntry();
    auto prevIndexEntry = _log.Get(prev_log_index);
    auto success = (lastEntry && lastEntry->term < term) && 
      (prevIndexEntry && prevIndexEntry->term == prev_log_term);
    return AppendResult{
      lastEntry ? lastEntry->term : 0,
      success
    };
  }
}
