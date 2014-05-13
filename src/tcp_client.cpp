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
          socket_(ios_) { 
            try {
              tcp_resolver resolver(ios_);
              tcp_resolver::query query(config_.host(), to_string(config_.port()));
              auto iterator = resolver.resolve(query);

              connect(socket_, iterator);
            } catch (const system_error& e) {
              cerr << e.what() << endl;
            }

            call_thread_ = shared_ptr<thread>(
                new thread([this](){ ios_.run();}));
          }

        ~impl();

        void append_entries_async(const append_entries_request&, appended_handler, timeout_handler);

      private:
        const config_server& config_;
        const timeout& timeout_;

        io_service ios_;
        io_service::work work_;
        tcp_socket socket_;
        shared_ptr<thread> call_thread_;
    };

    tcp::client::impl::~impl() {
      ios_.stop();
      try {
        call_thread_->join();
      } catch (const system_error& ) { /* suppress */ };
    };

    void tcp::client::impl::append_entries_async(const append_entries_request& r, appended_handler ah, timeout_handler th) {
      auto deadline = create_deadline(ios_, timeout_, th);
      auto handler = [this, ah, deadline, th]() {
        extend_deadline(deadline, timeout_, th);
        read_message<append_entries_response>(socket_, [deadline, ah, th](append_entries_response r) {
            deadline->cancel();
            ah(r);
          });
      };
      write_message<append_entries_request>(socket_, r, handler);
    }

    tcp::client::client(const config_server& c, const timeout& t) :
      pimpl_(new tcp::client::impl(c, t)) {} 

    void tcp::client::append_entries_async(const append_entries_request& r, appended_handler ah, timeout_handler th) {
      pimpl_->append_entries_async(r, ah, th);
    }

    tcp::client::~client() = default;
  }
}
