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

    struct tcp::client::impl {
      public:
        impl(const config_server& config, on_appended_handler ah, on_voted_handler vh) :
          ah_(ah),
          vh_(vh),
          ios_(),
          socket_(ios_),
          work_(ios_),
          threads_() { 
            for (size_t i = 0; i < threads_.size(); i++) {
              threads_[i] = shared_ptr<thread>(
                  new thread([this](){ ios_.run();}));
            }

            LOG_TRACE << "Client - connecting to " << config.host() << ":" << config.port();
            tcp_resolver resolver(socket_.get_io_service());
            tcp_resolver::query query(config.host(), to_string(config.port()));
            auto iterator = resolver.resolve(query);

            connect(socket_, iterator);
          }

        ~impl();

        void append_entries_async(unique_ptr<append_entries_request>, error_handler);
        void request_vote_async(unique_ptr<vote_request>, error_handler);

      private:
        void raft_message_handler(raft_message);

        on_appended_handler ah_;
        on_voted_handler vh_;

        io_service ios_;
        tcp_socket socket_;
        io_service::work work_;
        array<shared_ptr<thread>, 1> threads_;
    };

    tcp::client::impl::~impl() {
      socket_.close();
      ios_.stop();
      for (size_t i = 0; i < threads_.size(); i++) {
        try {
          threads_[i]->join();
        } catch (const system_error& ) { /* suppress */ };
      }
      LOG_TRACE  << "Client - connection closed.";
    };

    void tcp::client::impl::append_entries_async(unique_ptr<append_entries_request> request, error_handler eh) {
      // TODO protobuf performance
      raft_message m;
      m.set_discriminator(raft_message::APPEND_ENTRIES);
      m.set_allocated_append_entries_request(request.release());

      write_message_async(socket_, m, [](){}, eh);
      read_message_async(socket_, std::bind(&impl::raft_message_handler, this, _1), eh);
    }

    void tcp::client::impl::request_vote_async(unique_ptr<vote_request> request, error_handler eh) {
      // TODO protobuf performance
      raft_message m;
      m.set_discriminator(raft_message::VOTE);
      m.set_allocated_vote_request(request.release());

      write_message_async(socket_, m, [](){}, eh);
      read_message_async(socket_, std::bind(&impl::raft_message_handler, this, _1), eh);
    }

    void tcp::client::impl::raft_message_handler(raft_message m){
      switch(m.discriminator()) {
        case raft_message::APPEND_ENTRIES: {
          ah_(m.append_entries_response()); break;
        }
        case raft_message::VOTE: {
          vh_(m.vote_response()); break;
        }
      }
    }

    tcp::client::client(const config_server& c, on_appended_handler ah, on_voted_handler vh) :
      pimpl_(new tcp::client::impl(c, ah, vh)) {} 

    void tcp::client::append_entries_async(unique_ptr<append_entries_request> r, error_handler eh) {
      pimpl_->append_entries_async(move(r), eh);
    }

    void tcp::client::request_vote_async(unique_ptr<vote_request> r, error_handler eh) {
      pimpl_->request_vote_async(move(r), eh);
    }

    tcp::client::~client() = default;
  }
}
