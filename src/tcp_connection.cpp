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
      auto handler = [this] (const raft_message& m) {
        LOG_TRACE << "Server - message read:" << m.discriminator();
        // TODO protobuf performance
        response_.set_discriminator(m.discriminator());
        switch (m.discriminator()) {
          case raft_message::APPEND_ENTRIES:
            {
              auto r = append_handler_(m.append_entries_request());
              response_.set_allocated_append_entries_response(r.release());
              break;
            }
          case raft_message::VOTE:
            {
              auto r = vote_handler_(m.vote_request());
              response_.set_allocated_vote_response(r.release());
              break;
            }
          default:
            break;
        }
        auto write_handler = [this] () {
          LOG_TRACE << "Server - message written:" << response_.discriminator();
          start();
        };
        write_message_async(socket_, response_, write_handler, bind(&tcp_connection::write_error_handler, this, _1));
      };
      read_message_async(socket_, handler, bind(&tcp_connection::read_error_handler, this, _1));
    }

    void tcp_connection::read_error_handler(const boost::system::error_code& ec) {
      if (ec == boost::asio::error::operation_aborted) {
        LOG_WARN << "Read operation aborted. Connection can be disposed.";
      } else {
        // TODO: reconnect
        LOG_ERROR << "Error reading from socket: " << ec.value() << " - " << ec.message();
      }
    }

    void tcp_connection::write_error_handler(const boost::system::error_code& ec) {
      if (ec == boost::asio::error::operation_aborted) {
        LOG_WARN << "Write operation aborted. Connection can be disposed.";
      } else {
        // TODO: reconnect
        LOG_ERROR << "Error writing to socket: " << ec.value() << " - " << ec.message();
      }
    }

    void tcp_connection::close(){
      boost::system::error_code ignored_ec;
      socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
      socket_.close();
      LOG_INFO << "Server - connection closed";
    }
  }
}
