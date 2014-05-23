#include <iostream>

#include "tcp_connection.h"
#include "tcp_util.h"
#include "logging.h"

namespace raft {
  namespace rpc {

    using namespace std;
    using namespace std::placeholders;
    using namespace boost::asio;
    using namespace boost::system;

    void tcp_connection::start() {
      LOG_INFO << "Connection started";
      auto self = shared_from_this();
      auto handler = [this, self] (const append_entries_request& r) {
        response_ = handler_(r);
        auto write_handler = [this, self] () {
          error_code ignored_ec;
          socket_.shutdown(tcp_socket::shutdown_both, ignored_ec);
        };
        write_message<append_entries_response>(socket_, response_, write_handler);
      };
      read_message<append_entries_request>(socket_, handler);
    }
  }
}
