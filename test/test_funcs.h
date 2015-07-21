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

  class append_args {
    public:
      append_args(unsigned int term) : args_() {
        args_.set_term(term);
      }

      append_args& prev_index_term(unsigned int prevIndex, unsigned int prevTerm) {
        args_.set_prev_log_index(prevIndex);
        args_.set_prev_log_term(prevTerm);
        return *this;
      }

      append_args& commit_index(unsigned int index) {
        args_.set_leader_commit(index);
        return *this;
      }

      append_entries_request get() const {
        return args_;
      }

    private:
      append_entries_request args_;
  };

}
#endif
