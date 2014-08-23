#ifndef RAFT_RPC_UTIL
#define RAFT_RPC_UTIL

#include <boost/asio.hpp>
#include "rpc.h"

namespace raft {
  namespace rpc {

    typedef boost::asio::ip::tcp::socket tcp_socket;

    const static int TCP_HEADER_LENGTH = 4;

    void write_message_async(tcp_socket& s, const raft_message& request, std::function<void()> h, error_handler eh);
    void read_message_async(tcp_socket& s, std::function<void(const raft_message&)> h, error_handler eh);

    std::shared_ptr<boost::asio::deadline_timer> create_deadline(boost::asio::io_service& ios, timeout t, error_handler h);
    void extend_deadline(std::shared_ptr<boost::asio::deadline_timer> timer, timeout t, error_handler h);
  }
}
#endif
