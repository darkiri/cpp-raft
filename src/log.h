#ifndef RAFT_LOG
#define RAFT_LOG

#include <vector>
#include <memory>

using namespace std;

namespace raft {
  struct LogEntry {
    int term;
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
        return _entries.empty()
          ? nullptr
          : _entries.back().get();
      };
      const LogEntry* Get(unsigned int index) const {
        return _entries.size() <= index 
          ? nullptr 
          : _entries[index].get();
      }
      virtual unsigned int Size() const {
        return _entries.size();
      }
      void Append(const LogEntry& entry) {
        auto e = unique_ptr<LogEntry>(new LogEntry(entry));
        _entries.push_back(move(e));
      }
      void Trim(unsigned int index) {
        _entries.erase(_entries.begin() + index, _entries.end());
      }
      virtual ~InMemoryLog() { };
    private:
      vector<unique_ptr<LogEntry>> _entries;
  };
}
#endif
