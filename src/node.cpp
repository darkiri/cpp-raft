#include "node.h"

namespace raft {
    const AppendResult Node::AppendEntry(const Entry& e) {
        auto lastEntry = _log.GetLastEntry();
        auto success = e.term < lastEntry.term ? false : true;
        return AppendResult{lastEntry.term, success};
    }
}
