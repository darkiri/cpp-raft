#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <functional>
#include <boost/asio.hpp>

#include "rpc.h"
#include "tcp_util.h"

namespace raft {
  namespace rpc {

    using namespace std;
    using namespace std::placeholders;
    using namespace boost::asio;
    using namespace boost::system;

    typedef boost::asio::ip::tcp::resolver tcp_resolver;

    struct tcp::client::impl {
      public:
        impl(const config_server& c, const timeout& t) :
          config_(c),
          timeout_(t),
          ios_(),
          work_(ios_),
          socket_(ios_),
          threads_(){ 
            tcp_resolver resolver(ios_);
            tcp_resolver::query query(config_.host(), to_string(config_.port()));
            auto iterator = resolver.resolve(query);

            connect(socket_, iterator);

            for (size_t i = 0; i < threads_.size(); i++) {
              threads_[i] = shared_ptr<thread>(
                  new thread([this](){ ios_.run();}));
            }
          }

        ~impl();

        void append_entries_async(const append_entries_request&, on_appended_handler, error_handler);
        void request_vote_async(const vote_request&, on_voted_handler, error_handler);

      private:
        const config_server& config_;
        const timeout& timeout_;

        io_service ios_;
        io_service::work work_;
        tcp_socket socket_;
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

    void tcp::client::impl::append_entries_async(const append_entries_request& r, on_appended_handler ah, error_handler th) {
      auto deadline = create_deadline(ios_, timeout_, th);
      auto handler = [this, ah, deadline, th]() {
        extend_deadline(deadline, timeout_, th);
        read_message<raft_message>(socket_, [deadline, ah, th](raft_message m) {
            deadline->cancel();
            ah(m.append_entries_response());
          }, th);
      };
      raft_message m;
      m.set_discriminator(raft_message::APPEND_ENTRIES);
      auto newRequest = new append_entries_request();
      newRequest->MergeFrom(r);
      m.set_allocated_append_entries_request(newRequest);
      write_message<raft_message>(socket_, m, handler, th);
    }

    void tcp::client::impl::request_vote_async(const vote_request& r, on_voted_handler vh, error_handler eh) {
      auto deadline = create_deadline(ios_, timeout_, eh);
      auto handler = [this, vh, deadline, eh]() {
        extend_deadline(deadline, timeout_, eh);
        read_message<raft_message>(socket_, [deadline, vh, eh](raft_message m) {
            deadline->cancel();
            vh(m.vote_response());
          }, eh);
      };
      raft_message m;
      m.set_discriminator(raft_message::VOTE);
      auto newRequest = new vote_request();
      newRequest->MergeFrom(r);
      m.set_allocated_vote_request(newRequest);
      write_message<raft_message>(socket_, m, handler, eh);
    }

    tcp::client::client(const config_server& c, const timeout& t) :
      pimpl_(new tcp::client::impl(c, t)) {} 

    void tcp::client::append_entries_async(const append_entries_request& r, on_appended_handler ah, error_handler th) {
      pimpl_->append_entries_async(r, ah, th);
    }

    void tcp::client::request_vote_async(const vote_request& r, on_voted_handler vh, error_handler eh) {
      pimpl_->request_vote_async(r, vh, eh);
    }

    tcp::client::~client() = default;
  }
}
