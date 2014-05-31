#ifndef RAFT_LOG
#define RAFT_LOG

#include <vector>
#include <memory>
#include <algorithm>
#include "proto/raft.pb.h"

namespace raft {
  typedef std::vector<log_entry>::const_iterator const_iterator;
  class InMemoryLog {
      public:
        InMemoryLog() {};

        const_iterator Begin() const{
          return entries_.begin();
        }
        const_iterator End() const{
          return entries_.end();
        }
        unsigned int Size() const {
          return entries_.size();
        }
        void Append(const log_entry& e) {
          entries_.push_back(e);
        }
        void Trim(const_iterator iter) {
          auto  nonConstIter = entries_.begin() + (iter - entries_.begin());
          entries_.erase(nonConstIter, entries_.end());
        }
      private:
        InMemoryLog(const InMemoryLog&);
        InMemoryLog& operator=(const InMemoryLog&);

        std::vector<log_entry> entries_;
    };
  }
#endif
