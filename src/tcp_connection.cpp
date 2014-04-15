#include <iostream>

#include "tcp_connection.h"

namespace raft {
  namespace rpc {

    using namespace std;
    using namespace std::placeholders;
    using namespace boost::system;

    void tcp_connection::start(){
        auto res = handler_(append_entries_request());
        boost::asio::streambuf b;
        ostream os(&b);
        res.SerializeToOstream(&os);
        auto handler = bind(&tcp_connection::handle_write, this, _1, _2);
        async_write(socket_, b, handler);
    }

    void tcp_connection::handle_write(const error_code& error, size_t /*bytes_transferred*/) {
      if (!error) {
        error_code ignored_ec;
        socket_.shutdown(tcp_socket::shutdown_both, ignored_ec);
      } else {
        cerr << "Error writing to socket: " << error.value() << " - " << error.message() << endl;
      }
    }

  }
}
