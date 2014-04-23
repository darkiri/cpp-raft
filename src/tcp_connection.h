#ifndef RAFT_TCP_CONNECTION
#define RAFT_TCP_CONNECTION

#include <memory>
#include <boost/asio.hpp>

#include "rpc.h"

namespace raft {
  namespace rpc {
    typedef boost::asio::ip::tcp::endpoint tcp_endpoint;
    typedef boost::asio::ip::tcp::acceptor tcp_acceptor;
    typedef boost::asio::ip::tcp::socket tcp_socket;

    class tcp_connection : public std::enable_shared_from_this<tcp_connection> {
      public:
        static std::shared_ptr<tcp_connection> create(tcp_socket socket, append_handler h) {
          return std::shared_ptr<tcp_connection>(new tcp_connection(std::move(socket), h));
        }
        tcp_socket& socket() {
          return socket_;
        }
        void start();
      private:
        tcp_connection(tcp_socket socket, append_handler h) :
          socket_(std::move(socket)),
          handler_(h) {}

        tcp_socket socket_;
        append_handler handler_;
    };
  }
}
#endif
