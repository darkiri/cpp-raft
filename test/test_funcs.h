#ifndef test_funcs
#define test_funcs

#include "node.h"

namespace raft {
  inline LogEntry CreateLogEntry(int term) {
    return LogEntry { term };
  }
}
#endif
