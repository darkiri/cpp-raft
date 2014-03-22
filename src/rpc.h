#ifndef RAFT_RPC
#define RAFT_RPC

#include "config.h"
#include "timeout.h"
#include "node.h"

namespace raft {
  namespace rpc {
    typedef std::function<const AppendEntriesRes(const AppendEntriesArgs&)> append_entries_handler;
    typedef std::function<const AppendEntriesRes(const RequestVoteArgs&)> request_vote_handler;

    class tcp_server {
      public:
        tcp_server(const config& c,
            append_entries_handler leader_append_handler,
            append_entries_handler append_handler,
            request_vote_handler request_vote_handler) : 
          peers_(c),
          leader_append_handler_(leader_append_handler),
          append_handler_(append_handler),
          request_vote_handler_(request_vote_handler) {}
        tcp_server(const tcp_server&) = delete;
        tcp_server(tcp_server&&) = delete;

        void run();
        void stop();
      private:
        const config& peers_;
        append_entries_handler leader_append_handler_;
        append_entries_handler append_handler_;
        request_vote_handler request_vote_handler_;
    };

    class tcp_client {
      public:
        tcp_client(const config& c,
            const timeout& t) :
          config_(c),
          timeout_(t) {}
        tcp_client(const tcp_client&) = delete;
        tcp_client(tcp_client&&) = delete;

        void append_entries_async(const AppendEntriesArgs&, std::function<void(const AppendEntriesRes&)>);
        void request_vote_async(const RequestVoteArgs&, std::function<void(const AppendEntriesRes&)>);
      private:
        const config& config_;
        const timeout& timeout_;
    };
  }
}
#endif
