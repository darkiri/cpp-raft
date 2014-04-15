#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <functional>

#include "rpc.h"

namespace raft {
  namespace rpc {

    using namespace std;
    using namespace std::placeholders;
    using namespace boost::asio;
    using namespace boost::system;

    typedef boost::asio::ip::tcp::endpoint tcp_endpoint;
    typedef boost::asio::ip::tcp::acceptor tcp_acceptor;
    typedef boost::asio::ip::tcp::socket tcp_socket;

    struct tcp::server::impl {
      public:
        impl(const config_server& c, append_handler h) : 
          config_(c),
          handler_(h),
          thread_pool_(),
          ios_(),
          acceptor_(ios_),
          socket_(ios_) {
            auto e = tcp_endpoint(ip::tcp::v4(), config_.port());
            acceptor_.open(e.protocol());
            acceptor_.set_option(tcp_acceptor::reuse_address(true));
            acceptor_.bind(e);
            acceptor_.listen();
          }

        void run();
        void stop();

      private:
        static const std::size_t THREAD_POOL_SIZE = 2;

        void start_accept();
        void handle_accept(shared_ptr<tcp_connection>, const error_code&);

        const config_server& config_;
        append_handler handler_;
        array<shared_ptr<thread>, THREAD_POOL_SIZE> thread_pool_;

        io_service ios_;
        tcp_acceptor acceptor_;
        tcp_socket socket_;
    };

    class tcp_connection {
      public:
        static shared_ptr<tcp_connection> create(tcp_socket socket) {
          return shared_ptr<tcp_connection>(new tcp_connection(move(socket)));
        }
        tcp_socket& socket() {
          return socket_;
        }
      private:
        tcp_connection(tcp_socket socket): socket_(move(socket)) {}
        tcp_socket socket_;
    };

    void tcp::server::impl::run() {
      start_accept();
      try {
        for (size_t i = 0; i < THREAD_POOL_SIZE; i++) {
          thread_pool_[i] = shared_ptr<thread>(
              new thread([this](){ ios_.run();}));
        }
      } catch (exception& e) {
        cerr << e.what() << endl;
      }
    }

    void tcp::server::impl::stop() {
      ios_.stop();
      for (size_t i = 0; i < THREAD_POOL_SIZE; i++) {
        thread_pool_[i]->join();
      }
    }

    void tcp::server::impl::start_accept(){
      auto conn = tcp_connection::create(move(socket_));
      auto handler = bind(&impl::handle_accept, this, conn, _1);
      acceptor_.async_accept(conn->socket(), handler);
    }

    void handle_write(const error_code& error, size_t /*bytes_transferred*/) {
      if (error) {
        cerr << "Error writing to socket: " << error.value() << " - " << error.message() << endl;
      }
    }

    void tcp::server::impl::handle_accept(
        shared_ptr<tcp_connection> conn,
        const error_code& error) {
      if (!error) {
        auto res = handler_(append_entries_request());
        boost::asio::streambuf b;
        ostream os(&b);
        res.SerializeToOstream(&os);
        auto handler = bind(handle_write, _1, _2);
        async_write(conn->socket(), b, handler);
        start_accept();
      } else {
        std::cerr << "Error accepting connection: " << error.value() << " - " << error.message() << endl;
      }
    }

    tcp::server::server(const config_server& c, append_handler h) :
      pimpl_(new tcp::server::impl(c, h)) {} 

    void tcp::server::run() {
      pimpl_->run();
    }

    void tcp::server::stop() {
      pimpl_->stop();
    }

    tcp::server::~server() = default;
  }
}
