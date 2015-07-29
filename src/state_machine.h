#ifndef STATE_MACHING
#define STATE_MACHING

#include "proto/raft.pb.h"

namespace raft {
  class state_machine {
    public:
      void apply(const log_entry& e) {
        if (nullptr == pfirst_applied_) {
          pfirst_applied_ = &e;
        }
        plast_applied_ = &e;
      }

      const log_entry* first_applied() const {
        return pfirst_applied_;
      }

      const log_entry* last_applied() const {
        return plast_applied_;
      }
    private:
      const log_entry* pfirst_applied_;
      const log_entry* plast_applied_;
  };
}
#endif
