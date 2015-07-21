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

  class init_log{
    public:
      init_log(in_memory_log& log, unsigned int currentTerm) : log_(log) {
        log_.set_current_term(currentTerm);
      }

      init_log& entry(unsigned int entryTerm) {
        log_entry e;
        e.set_term(entryTerm);
        log_.append(e);
        return *this;
      }
    private:
      in_memory_log& log_;
  };

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

      append_args& log_entry(unsigned int entryTerm) {
        auto entry = args_.add_entries();
        entry->set_term(entryTerm);
        return *this;
      }

      append_entries_request get() const {
        return args_;
      }

    private:
      append_entries_request args_;
  };

  class vote_args {
    public:
      vote_args(unsigned int term) : args_() {
        args_.set_term(term);
      }

      vote_args& candidate(unsigned int id) {
        args_.set_candidate_id(id);
        return *this;
      }

      vote_args& last_index_term(unsigned int lastIndex, unsigned int lastTerm) {
        args_.set_last_log_index(lastIndex);
        args_.set_last_log_term(lastTerm);
        return *this;
      }

      vote_request get() const {
        return args_;
      }
    private:
      vote_request args_;
  };
}
#endif
