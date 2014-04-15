#include <iostream>
#include <thread>
#include <functional>

#include "rpc.h"
#include "tcp_connection.h"

namespace raft {
  namespace rpc {

    using namespace std;
    using namespace std::placeholders;
    using namespace boost::asio;
    using namespace boost::system;

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
      auto conn = tcp_connection::create(move(socket_), handler_);
      auto handler = bind(&impl::handle_accept, this, conn, _1);
      acceptor_.async_accept(conn->socket(), handler);
    }

    void tcp::server::impl::handle_accept(
        shared_ptr<tcp_connection> conn,
        const error_code& error) {
      if (!error) {
        conn->start();
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
