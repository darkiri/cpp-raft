#ifndef RAFT_RPC
#define RAFT_RPC

#include <memory>

#include "proto/raft.pb.h"
#include "timeout.h"

namespace raft {
  namespace rpc {
    typedef std::function<append_entries_response(const append_entries_request&)> append_handler;

    class tcp {
      public:
        class server {
          public:
            server(const config_server& c, append_handler h); 
            ~server();

            void run();
            void stop();

          private:
            struct impl;
            std::unique_ptr<impl> pimpl_;
        };

        class client {
          public:
            client(const config& c, const timeout& t) :
              config_(c), timeout_(t) {}

            client(const client&) = delete;
            client(client&&) = delete;

            void append_entries_async(const append_entries_request&, std::function<void(const append_entries_response&)>);
          private:
            const config& config_;
            const timeout& timeout_;
        };
    };
  }
}
#endif
