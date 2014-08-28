#ifndef RAFT_TCP_CONNECTION
#define RAFT_TCP_CONNECTION

#include <memory>
#include <boost/asio.hpp>

#include "logging.h"
#include "rpc.h"

namespace raft {
  namespace rpc {
    typedef boost::asio::ip::tcp::socket tcp_socket;

    class tcp_connection {
      public:
        static std::shared_ptr<tcp_connection> create(tcp_socket socket, append_handler ah, vote_handler vh, error_handler th) {
          return std::shared_ptr<tcp_connection>(new tcp_connection(std::move(socket), ah, vh, th));
        }
        tcp_socket& socket() {
          return socket_;
        }
        void start();
        void close();
      private:
        tcp_connection(tcp_socket socket, append_handler ah, vote_handler vh, error_handler th) :
          socket_(std::move(socket)),
          append_handler_(ah),
          vote_handler_(vh),
          error_handler_(th),
          response_() {
            LOG_TRACE << "Server - connection created";
          }
        void read_error_handler(const boost::system::error_code&);
        void write_error_handler(const boost::system::error_code&);

        tcp_socket socket_;
        append_handler append_handler_;
        vote_handler vote_handler_;
        error_handler error_handler_;
        raft_message response_;
    };
  }
}
#endif
