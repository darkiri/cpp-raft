#ifndef RAFT_LOG
#define RAFT_LOG

#include <vector>
#include <memory>
#include <algorithm>

namespace raft {
  struct LogEntry {
    unsigned int term;
    // TODO payload
  };

  typedef std::vector<LogEntry>::const_iterator InMemoryIterator;
  class InMemoryLog {
      public:
        InMemoryLog() {};

        InMemoryIterator Begin() const{
          return entries_.begin();
        }
        InMemoryIterator End() const{
          return entries_.end();
        }
        unsigned int Size() const {
          return entries_.size();
        }
        void Append(InMemoryIterator begin, InMemoryIterator end) {
          std::for_each(begin, end, [this](const LogEntry& e) { entries_.push_back(e);});
        }
        void Append(const LogEntry& e) {
          entries_.push_back(e);
        }
        void Trim(InMemoryIterator iter) {
          auto  nonConstIter = entries_.begin() + (iter - entries_.begin());
          entries_.erase(nonConstIter, entries_.end());
        }
      private:
        InMemoryLog(const InMemoryLog&);
        InMemoryLog& operator=(const InMemoryLog&);

        std::vector<LogEntry> entries_;
    };
  }
#endif
