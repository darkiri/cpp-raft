#ifndef RAFT_TCP_CONNECTION
#define RAFT_TCP_CONNECTION

#include <memory>
#include <boost/asio.hpp>

#include "rpc.h"

namespace raft {
  namespace rpc {
    typedef boost::asio::ip::tcp::socket tcp_socket;

    class tcp_connection : public std::enable_shared_from_this<tcp_connection> {
      public:
        static std::shared_ptr<tcp_connection> create(tcp_socket socket, const timeout& t, append_handler h, error_handler th) {
          return std::shared_ptr<tcp_connection>(new tcp_connection(std::move(socket), t, h, th));
        }
        tcp_socket& socket() {
          return socket_;
        }
        void start();
      private:
        tcp_connection(tcp_socket socket, const timeout& t, append_handler h, error_handler th) :
          socket_(std::move(socket)),
          timeout_(t),
          handler_(h),
          error_handler_(th),
          response_() {}

        tcp_socket socket_;
        const timeout timeout_;
        append_handler handler_;
        error_handler error_handler_;
        append_entries_response response_;
    };
  }
}
#endif
