#ifndef test_funcs
#define test_funcs

#include "node.h"
#include "proto/raft.pb.h"

namespace raft {
  inline log_entry create_log_entry(unsigned int term) {
    log_entry l;
    l.set_term(term);
    return l;
  }
  inline void init_log_entry(log_entry* l, unsigned int term) {
    l->set_term(term);
  }
}
#endif
