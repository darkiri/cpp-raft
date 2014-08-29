#ifndef RAFT_TCP_CONNECTION
#define RAFT_TCP_CONNECTION

#include <memory>
#include <boost/asio.hpp>

#include "logging.h"
#include "rpc.h"

namespace raft {
  namespace rpc {
    typedef boost::asio::ip::tcp::socket tcp_socket;
    class tcp_connection;
    typedef std::function<void(tcp_connection&, const boost::system::error_code&)> connection_error_handler;

    class tcp_connection {
      public:
        static std::shared_ptr<tcp_connection> create(tcp_socket socket, append_handler ah, vote_handler vh, connection_error_handler eh) {
          return std::shared_ptr<tcp_connection>(new tcp_connection(std::move(socket), ah, vh, eh));
        }
        tcp_socket& socket() {
          return socket_;
        }
        void start();
        void close();
      private:
        tcp_connection(tcp_socket socket, append_handler ah, vote_handler vh, connection_error_handler th) :
          socket_(std::move(socket)),
          append_handler_(ah),
          vote_handler_(vh),
          error_handler_(th),
          response_() {
            LOG_TRACE << "Server - connection created";
          }
        void on_error(const boost::system::error_code&);

        tcp_socket socket_;
        append_handler append_handler_;
        vote_handler vote_handler_;
        connection_error_handler error_handler_;
        raft_message response_;
    };
  }
}
#endif
