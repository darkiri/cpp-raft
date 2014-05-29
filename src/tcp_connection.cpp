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
      auto deadline = create_deadline(socket_.get_io_service(), timeout_, error_handler_);
      auto self = shared_from_this();
      auto handler = [this, self, deadline] (const raft_message& r) {
        extend_deadline(deadline, timeout_, error_handler_);
        response_.set_discriminator(r.discriminator());
        switch (r.discriminator()) {
          case raft_message::APPEND_ENTRIES:
            {
              auto aer = append_handler_(r.append_entries_request());
              auto newAEResponse = new append_entries_response();
              newAEResponse->MergeFrom(aer);
              response_.set_allocated_append_entries_response(newAEResponse);
              break;
            }
          case raft_message::VOTE:
            {
              auto vr = vote_handler_(r.vote_request());
              auto newVResponse = new vote_response();
              newVResponse->MergeFrom(vr);
              response_.set_allocated_vote_response(newVResponse);
              break;
            }
          default:
            break;
        }
        auto write_handler = [this, self, deadline] () {
          deadline->cancel();
          error_code ignored_ec;
          socket_.shutdown(tcp_socket::shutdown_both, ignored_ec);
        };
        write_message<raft_message>(socket_, response_, write_handler, error_handler_);
      };
      read_message<raft_message>(socket_, handler, error_handler_);
    }
  }
}
