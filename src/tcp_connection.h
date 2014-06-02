#ifndef RAFT_TCP_CONNECTION
#define RAFT_TCP_CONNECTION

#include <memory>
#include <boost/asio.hpp>

#include "logging.h"
#include "rpc.h"

namespace raft {
  namespace rpc {
    typedef boost::asio::ip::tcp::socket tcp_socket;

    class tcp_connection : public std::enable_shared_from_this<tcp_connection> {
      public:
        static std::shared_ptr<tcp_connection> create(tcp_socket socket, const timeout& t, append_handler ah, vote_handler vh, error_handler th) {
          return std::shared_ptr<tcp_connection>(new tcp_connection(std::move(socket), t, ah, vh, th));
        }
        tcp_socket& socket() {
          return socket_;
        }
        void start();
        ~tcp_connection();
      private:
        tcp_connection(tcp_socket socket, const timeout& t, append_handler ah, vote_handler vh, error_handler th) :
          socket_(std::move(socket)),
          timeout_(t),
          append_handler_(ah),
          vote_handler_(vh),
          error_handler_(th),
          response_() {
            LOG_TRACE << "Server - connection created";
          }

        tcp_socket socket_;
        const timeout timeout_;
        append_handler append_handler_;
        vote_handler vote_handler_;
        error_handler error_handler_;
        raft_message response_;
    };
  }
}
#endif
