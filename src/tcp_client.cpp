#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <functional>
#include <boost/asio.hpp>

#include "rpc.h"
#include "tcp_util.h"
#include "logging.h"

namespace raft {
  namespace rpc {

    using namespace std;
    using namespace std::placeholders;
    using namespace boost::asio;
    using namespace boost::system;

    typedef boost::asio::ip::tcp::resolver tcp_resolver;

    class tcp_client_connection {
      public:
        tcp_client_connection(const config_server& config, tcp_socket socket, const timeout& t) :
          socket_(std::move(socket)),
          timeout_(t) {
            LOG_TRACE << "Client - connecting to " << config.host() << ":" << config.port();
            tcp_resolver resolver(socket_.get_io_service());
            tcp_resolver::query query(config.host(), to_string(config.port()));
            auto iterator = resolver.resolve(query);

            connect(socket_, iterator);
        }

        ~tcp_client_connection() {
          socket_.close();
          LOG_TRACE  << "Client - connection closed.";
        }

        void invoke_async(raft_message&, std::function<void(const raft_message&)> h, error_handler eh);

      private:
        tcp_client_connection(const tcp_client_connection&) = delete;
        tcp_client_connection& operator=(const tcp_client_connection&) = delete;

        tcp_socket socket_;
        timeout timeout_;
    };

    struct tcp::client::impl {
      public:
        impl(const config_server& c, const timeout& t) :
          config_(c),
          timeout_(t),
          ios_(),
          work_(ios_),
          threads_(){ 
            for (size_t i = 0; i < threads_.size(); i++) {
              threads_[i] = shared_ptr<thread>(
                  new thread([this](){ ios_.run();}));
            }
          }

        ~impl();

        void append_entries_async(unique_ptr<append_entries_request>, on_appended_handler, error_handler);
        void request_vote_async(unique_ptr<vote_request>, on_voted_handler, error_handler);

      private:
        const config_server& config_;
        const timeout& timeout_;

        io_service ios_;
        io_service::work work_;
        array<shared_ptr<thread>, 2> threads_;
    };

    tcp::client::impl::~impl() {
      ios_.stop();
      for (size_t i = 0; i < threads_.size(); i++) {
        try {
          threads_[i]->join();
        } catch (const system_error& ) { /* suppress */ };
      }
      LOG_INFO << "Client stopped.";
    };

    void tcp::client::impl::append_entries_async(unique_ptr<append_entries_request> r, on_appended_handler ah, error_handler th) {
      // TODO protobuf performance
      raft_message m;
      m.set_discriminator(raft_message::APPEND_ENTRIES);
      m.set_allocated_append_entries_request(r.release());

      auto conn = shared_ptr<tcp_client_connection>(new tcp_client_connection(config_, tcp_socket(ios_), timeout_));
      conn->invoke_async(m,
          [conn, ah](const raft_message& m){ ah(m.append_entries_response()); },
          [conn, th](){th();});
    }

    void tcp::client::impl::request_vote_async(unique_ptr<vote_request> r, on_voted_handler vh, error_handler th) {
      // TODO protobuf performance
      raft_message m;
      m.set_discriminator(raft_message::VOTE);
      m.set_allocated_vote_request(r.release());

      auto conn = shared_ptr<tcp_client_connection>(new tcp_client_connection(config_, tcp_socket(ios_), timeout_));
      conn->invoke_async(m,
          [conn, vh](const raft_message& m){ vh(m.vote_response()); },
          [conn, th](){th();});
    }

    void tcp_client_connection::invoke_async(raft_message& m, std::function<void(const raft_message&)> h, error_handler eh) {
      auto deadline = create_deadline(socket_.get_io_service(), timeout_, eh);
      auto handler = [this, h, deadline, eh]() {
        extend_deadline(deadline, timeout_, eh);
        read_message_async(socket_, [deadline, h, eh](raft_message m) {
            deadline->cancel();
            h(m);
          }, eh);
      };
      write_message_async(socket_, m, handler, eh);
    }

    tcp::client::client(const config_server& c, const timeout& t) :
      pimpl_(new tcp::client::impl(c, t)) {} 

    void tcp::client::append_entries_async(unique_ptr<append_entries_request> r, on_appended_handler ah, error_handler th) {
      pimpl_->append_entries_async(move(r), ah, th);
    }

    void tcp::client::request_vote_async(unique_ptr<vote_request> r, on_voted_handler vh, error_handler eh) {
      pimpl_->request_vote_async(move(r), vh, eh);
    }

    tcp::client::~client() = default;
  }
}
