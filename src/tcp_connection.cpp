#include <iostream>

#include "tcp_connection.h"
#include "tcp_util.h"
#include "logging.h"
#include <thread>

namespace raft {
  namespace rpc {

    using namespace std;
    using namespace std::placeholders;
    using namespace boost::asio;
    using namespace boost::system;

    void tcp_connection::start() {
      LOG_TRACE << "Server - connection accepted";
      auto deadline = create_deadline(socket_.get_io_service(), timeout_, error_handler_);
      auto self = shared_from_this();
      auto handler = [this, self, deadline] (const raft_message& m) {
        extend_deadline(deadline, timeout_, error_handler_);
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
        auto write_handler = [this, self, deadline] () {
          deadline->cancel();
          LOG_TRACE << "Server - shutdown connection";
          boost::system::error_code ignored_ec;
          socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
        };
        write_message_async(socket_, response_, write_handler, error_handler_);
      };
      read_message_async(socket_, handler, error_handler_);
    }

    tcp_connection::~tcp_connection(){
      socket_.close();
      LOG_TRACE << "Server - connection closed";
    }
  }
}
