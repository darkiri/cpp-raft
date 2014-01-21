#ifndef RAFT_LOG
#define RAFT_LOG

#include <vector>
#include <memory>

namespace raft {
  struct LogEntry {
    unsigned int term;
    // TODO payload
  };

  class InMemoryLog {
      public:
        InMemoryLog() {};

        std::vector<LogEntry>::const_iterator Begin() const{
          return entries_.begin();
        }
        std::vector<LogEntry>::const_iterator End() const{
          return entries_.end();
        }
        virtual unsigned int Size() const {
          return entries_.size();
        }
        void Append(const LogEntry& entry) {
          entries_.push_back(entry);
        }
        void Trim(unsigned int index) {
          entries_.erase(entries_.begin() + index, entries_.end());
        }
        virtual ~InMemoryLog() { };
      private:
        InMemoryLog(const InMemoryLog&);
        InMemoryLog& operator=(const InMemoryLog&);

        std::vector<LogEntry> entries_;
    };
  }
#endif
