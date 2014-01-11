#ifndef RAFT_LOG
#define RAFT_LOG

#include <vector>
#include <memory>

using namespace std;

namespace raft {
  struct LogEntry {
    unsigned int term;
// TODO   void* data;
  };

  class Log {
    public:
      Log() {};
      virtual ~Log() {};

      // the return value is available as long as the Log alive
      virtual const LogEntry* GetLastEntry() const = 0;
      virtual const LogEntry* Get(unsigned int index) const = 0;
      virtual unsigned int Size() const = 0;
      virtual void Append(const LogEntry& entry) = 0;
      virtual void Trim(unsigned int index) = 0;
    private:
      Log(Log&);
      Log& operator=(const Log&);
  };

  class InMemoryLog : public Log {
    public:
      InMemoryLog() {};
      const LogEntry* GetLastEntry() const {
        return entries_.empty()
          ? nullptr
          : entries_.back().get();
      };
      const LogEntry* Get(unsigned int index) const {
        return entries_.size() <= index 
          ? nullptr 
          : entries_[index].get();
      }
      virtual unsigned int Size() const {
        return entries_.size();
      }
      void Append(const LogEntry& entry) {
        auto e = unique_ptr<LogEntry>(new LogEntry(entry));
        entries_.push_back(move(e));
      }
      void Trim(unsigned int index) {
        entries_.erase(entries_.begin() + index, entries_.end());
      }
      virtual ~InMemoryLog() { };
    private:
      vector<unique_ptr<LogEntry>> entries_;
  };
}
#endif
