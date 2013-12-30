#ifndef RAFT_LOG
#define RAFT_LOG

#include <iostream>
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
      virtual void Append(const LogEntry& entry) = 0;
    private:
      Log(Log&);
      Log& operator=(const Log&);
  };
}
#endif
