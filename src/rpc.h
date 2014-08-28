#ifndef RAFT_RPC
#define RAFT_RPC

#include <memory>
#include <boost/system/error_code.hpp>

#include "proto/raft.pb.h"
#include "timeout.h"

namespace raft {
  namespace rpc {
    typedef std::function<std::unique_ptr<append_entries_response>(const append_entries_request&)> append_handler;
    typedef std::function<void(const append_entries_response&)> on_appended_handler;
    typedef std::function<std::unique_ptr<vote_response>(const vote_request&)> vote_handler;
    typedef std::function<void(const vote_response&)> on_voted_handler;
    typedef std::function<void(const boost::system::error_code&)> error_handler;

    class tcp {
      public:
        // NOTE: timeout handling is moved to the code that uses the server.
        class server {
          public:
            server(const config_server& c, append_handler, vote_handler); 
            ~server();

            void run();
            void stop();

          private:
            struct impl;
            std::unique_ptr<impl> pimpl_;
        };

        // single tcp connection per client
        // TODO: reconnect
        // NOTE: no timeout for the client.
        // all requests are asynchron, all responces do not depend directly on requests
        // timeout handling is moved to the code that uses the client.
        // TODO: timeout for connect?
        class client {
          public:
            client(const config_server&, on_appended_handler, on_voted_handler, error_handler);
            ~client();

            // these two methods may not be called simultaneously
            void append_entries_async(std::unique_ptr<append_entries_request>);
            void request_vote_async(std::unique_ptr<vote_request>);
          private:
            struct impl;
            std::unique_ptr<impl> pimpl_;
        };
    };
  }
}
#endif
