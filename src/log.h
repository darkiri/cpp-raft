#ifndef RAFT_LOG
#define RAFT_LOG

#include <vector>
#include <memory>
#include <algorithm>
#include "proto/raft.pb.h"

namespace raft {
  typedef std::vector<log_entry>::const_iterator const_iterator;
  class in_memory_log {
      public:
        in_memory_log() :
          entries_(),
          voted_for_() {};

        const_iterator begin() const{
          return entries_.begin();
        }
        const_iterator end() const{
          return entries_.end();
        }
        unsigned int size() const {
          return entries_.size();
        }
        void append(const log_entry& e) {
          entries_.push_back(e);
        }
        void trim(const_iterator iter) {
          auto  nonConstIter = entries_.begin() + (iter - entries_.begin());
          entries_.erase(nonConstIter, entries_.end());
        }
        unsigned int voted_for() {
          return voted_for_;
        }
        void set_voted_for(unsigned int candidate_id){
          voted_for_ = candidate_id;
        }
        unsigned int current_term() {
          return current_term_;
        }
        void set_current_term(unsigned int term){
          current_term_ = term;
        }
      private:
        in_memory_log(const in_memory_log&);
        in_memory_log& operator=(const in_memory_log&);

        std::vector<log_entry> entries_;
        unsigned int voted_for_;
        unsigned int current_term_;
    };
  }
#endif
