#ifndef RAFT_RPC
#define RAFT_RPC

#include <memory>

#include "proto/raft.pb.h"
#include "timeout.h"

namespace raft {
  namespace rpc {
    typedef std::function<std::unique_ptr<append_entries_response>(const append_entries_request&)> append_handler;
    typedef std::function<void(const append_entries_response&)> on_appended_handler;
    typedef std::function<std::unique_ptr<vote_response>(const vote_request&)> vote_handler;
    typedef std::function<void(const vote_response&)> on_voted_handler;
    typedef std::function<void()> error_handler;

    class tcp {
      public:
        class server {
          public:
            server(const config_server& c, const timeout&, append_handler, vote_handler, error_handler); 
            ~server();

            void run();
            void stop();

          private:
            struct impl;
            std::unique_ptr<impl> pimpl_;
        };

        class client {
          public:
            client(const config_server& c, const timeout& t);
            ~client();

            void append_entries_async(std::unique_ptr<append_entries_request>, on_appended_handler, error_handler);
            void request_vote_async(std::unique_ptr<vote_request>, on_voted_handler, error_handler);
          private:
            struct impl;
            std::unique_ptr<impl> pimpl_;
        };
    };
  }
}
#endif
