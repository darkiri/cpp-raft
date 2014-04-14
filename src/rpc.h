#ifndef RAFT_RPC
#define RAFT_RPC

#include "proto/raft.pb.h"
#include "timeout.h"
#include "node.h"

#include <array>
#include <thread>
#include <boost/asio.hpp>
#include <boost/asio.hpp>

namespace raft {
  namespace rpc {
    typedef std::function<append_entries_response(const append_entries_request&)> append_entries_handler;
    typedef boost::asio::ip::tcp::socket tcp_socket;
    typedef boost::asio::ip::tcp::acceptor tcp_acceptor;

    class tcp_connection;
    class tcp {
      public:
        class server {
          public:
            server(const config_server& c, append_entries_handler append_handler) : 
              config_(c),
              append_handler_(append_handler),
              io_service_(),
              acceptor_(io_service_),
              socket_(io_service_){
                auto e = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), config_.port());
                acceptor_.open(e.protocol());
                acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
                acceptor_.bind(e);
                acceptor_.listen();
              }

            server(const server&) = delete;
            server(server&&) = delete;

            void run();
            void stop();

          private:
            static const std::size_t THREAD_POOL_SIZE = 2;

            void start_accept();
            void handle_accept(
                std::shared_ptr<tcp_connection> s,
                const boost::system::error_code&);

            const config_server& config_;
            append_entries_handler append_handler_;

            boost::asio::io_service io_service_;
            tcp_acceptor acceptor_;
            tcp_socket socket_;
            std::array<std::shared_ptr<std::thread>, THREAD_POOL_SIZE> thread_pool_;
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
