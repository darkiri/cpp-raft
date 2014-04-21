#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <functional>
#include <boost/asio.hpp>

#include "rpc.h"

namespace raft {
  namespace rpc {

    using namespace std;
    using namespace std::placeholders;
    using namespace boost::asio;
    using namespace boost::system;

    typedef boost::asio::ip::tcp::resolver tcp_resolver;
    typedef boost::asio::ip::tcp::socket tcp_socket;

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
        void handle_write(const error_code& error, size_t /*bytes_transferred*/, on_appended_handler);
        void handle_read(const error_code& error, shared_ptr<char> payload, int size, on_appended_handler h);

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
      auto data = pack(r);
      auto handler = [this, h, data] (const error_code& ec, size_t s) {
        handle_write(ec, s, h);
      };
      async_write(socket_, buffer(data.get(), get_size(r)), handler);
    }

    void tcp::client::impl::handle_write(const error_code& error, size_t /*bytes_transferred*/, on_appended_handler h) {
      if (!error) {
        cout << "written" << endl;
        array<char, HEADER_LENGTH> data;
        read(socket_, buffer(&data, HEADER_LENGTH));
        size_t size = deserialize_int(data);
        cout <<  "payload size: " << size << endl;
        auto payload = shared_ptr<char>(new char[size]);
        auto handler = [this, payload, h] (const error_code& ec, size_t size) {
          this->handle_read(ec, payload, size, h);
        };
        async_read(socket_, buffer(payload.get(), size), handler);
      } else {
        cerr << "Error writing to socket: " << error.value() << " - " << error.message() << endl;
      }
    }

    void tcp::client::impl::handle_read(const error_code& error, shared_ptr<char> payload, int size, on_appended_handler h) {
      if (!error) {
        cout << "handle read " << size << " bytes" << endl;
        append_entries_response res;
        res.ParseFromArray(payload.get(), size);
        h(res);
      } else {
        cerr << "Error reading from socket: " << error.value() << " - " << error.message() << endl;
      }
    }
    tcp::client::client(const config_server& c, const timeout& t) :
      pimpl_(new tcp::client::impl(c, t)) {} 

    void tcp::client::append_entries_async(const append_entries_request& r, on_appended_handler h) {
      pimpl_->append_entries_async(r, h);
    }

    tcp::client::~client() = default;
  }
}
