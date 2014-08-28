#include <iostream>
#include <thread>
#include <functional>

#include "rpc.h"
#include "tcp_connection.h"
#include "tcp_util.h"

namespace raft {
  namespace rpc {

    using namespace std;
    using namespace std::placeholders;
    using namespace boost::asio;
    using namespace boost::system;

    typedef boost::asio::ip::tcp::endpoint tcp_endpoint;
    typedef boost::asio::ip::tcp::acceptor tcp_acceptor;

    struct tcp::server::impl {
      public:
        impl(const config_server& c, append_handler ah, vote_handler vh, error_handler th) : 
          config_(c),
          append_handler_(ah),
          vote_handler_(vh),
          error_handler_(th),
          thread_pool_(),
          ios_(),
          acceptor_(ios_),
          socket_(ios_),
          connection_pool_() {
            auto e = tcp_endpoint(ip::tcp::v4(), config_.port());
            acceptor_.open(e.protocol());
            acceptor_.set_option(tcp_acceptor::reuse_address(true));
            acceptor_.bind(e);
            acceptor_.listen();
          }

        void run();
        void stop();

      private:
        static const std::size_t THREAD_POOL_SIZE = 1;

        void start_accept();
        void handle_accept(shared_ptr<tcp_connection>, const error_code&);

        const config_server& config_;

        append_handler append_handler_;
        vote_handler vote_handler_;
        error_handler error_handler_;
        array<shared_ptr<thread>, THREAD_POOL_SIZE> thread_pool_;

        io_service ios_;
        tcp_acceptor acceptor_;
        tcp_socket socket_;
        vector<shared_ptr<tcp_connection>> connection_pool_;
    };

    void tcp::server::impl::run() {
      start_accept();
      for (size_t i = 0; i < THREAD_POOL_SIZE; i++) {
        thread_pool_[i] = shared_ptr<thread>(
            new thread([this](){
              try { ios_.run(); } 
              catch (const system_error& e) {
              LOG_ERROR << "Exception in the Server infrastructure. The server is dead. Details: " << e.what();
            }}));
      }
    }

    void tcp::server::impl::stop() {
      for (auto conn : connection_pool_) {
        conn->close();
      }
      ios_.stop();
      for (size_t i = 0; i < THREAD_POOL_SIZE; i++) {
        try {
          thread_pool_[i]->join();
        } catch (const system_error& ) { /* suppress */ };
      }
      LOG_INFO << "Server stopped.";
    }

    void tcp::server::impl::start_accept(){
      auto conn = tcp_connection::create(move(socket_), append_handler_, vote_handler_, error_handler_);
      // TODO: connection monitoring, kill timed out connections
      connection_pool_.push_back(conn);
      auto handler = bind(&impl::handle_accept, this, conn, _1);
      acceptor_.async_accept(conn->socket(), handler);
    }

    void tcp::server::impl::handle_accept(shared_ptr<tcp_connection> conn, const error_code& error) {
      if (!error) {
        LOG_INFO << "Server - connection accepted";
        conn->start();
        start_accept();
      } else {
        LOG_ERROR << "Error accepting connection: " << error.value() << " - " << error.message() << endl;
      }
    }

    tcp::server::server(const config_server& c, append_handler ah, vote_handler vh, error_handler th) :
      pimpl_(new tcp::server::impl(c, ah, vh, th)) {} 

    void tcp::server::run() {
      pimpl_->run();
    }

    void tcp::server::stop() {
      pimpl_->stop();
    }

    tcp::server::~server() = default;
  }
}
