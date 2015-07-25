#ifndef STATE_MACHING
#define STATE_MACHING

#include "proto/raft.pb.h"

namespace raft {
  class state_machine {
    public:
      void apply(const log_entry& e) {
        plast_applied_ = &e;
      }
    private:
      const log_entry* plast_applied_;
  };
}
#endif
