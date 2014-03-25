#ifndef RAFT_RPC
#define RAFT_RPC

#include "proto/raft.pb.h"
#include "timeout.h"
#include "node.h"

#include <boost/asio.hpp>

namespace raft {
  namespace rpc {
    typedef std::function<append_entries_response(const append_entries_request&)> append_entries_handler;

    namespace tcp1 {
      class server {
        public:
          server(const config_server& c,
              append_entries_handler append_handler) : 
            config_(c),
            append_handler_(append_handler),
            io_service_(),
            socket_(io_service_),
            acceptor_(io_service_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 7575)){}

          server(const server&) = delete;
          server(server&&) = delete;

          void run();
          void stop();
        private:
          void start_accept();
          void handle_accept(const boost::system::error_code&);
          void handle_write(const boost::system::error_code&, size_t);

          const config_server& config_;
          append_entries_handler append_handler_;

          boost::asio::io_service io_service_;
          boost::asio::ip::tcp::socket socket_;
          boost::asio::ip::tcp::acceptor acceptor_;
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
    }
  }
}
#endif
