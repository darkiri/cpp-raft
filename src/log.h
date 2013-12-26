#ifndef RAFT_LOG
#define RAFT_LOG

namespace raft {
    struct LogEntry {
        int term;
        void* data;
    };

    class Log {
        public:
            Log() {};
            virtual ~Log() {};

            virtual const LogEntry& GetLastEntry() const = 0;
        private:
            Log(Log&);
            Log& operator=(const Log&);
    };
}
#endif
