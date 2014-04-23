#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <functional>
#include <boost/asio.hpp>

#include "rpc.h"
#include "rpc_util.h"

namespace raft {
  namespace rpc {

    using namespace std;
    using namespace std::placeholders;
    using namespace boost::asio;
    using namespace boost::system;

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

        void append_entries_async(const append_entries_request&, on_appended_handler);

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

    void tcp::client::impl::append_entries_async(const append_entries_request& r, on_appended_handler h) {
      auto handler = [this, h](const append_entries_request&) {
        read_message<append_entries_response>(socket_, h);
      };
      write_message<append_entries_request>(socket_, r, handler);
    }

    tcp::client::client(const config_server& c, const timeout& t) :
      pimpl_(new tcp::client::impl(c, t)) {} 

    void tcp::client::append_entries_async(const append_entries_request& r, on_appended_handler h) {
      pimpl_->append_entries_async(r, h);
    }

    tcp::client::~client() = default;
  }
}
