#include <iostream>
#include "rpc.h"
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <thread>
#include <functional>

using namespace std;
using namespace std::placeholders;
using namespace boost::asio;
using namespace boost::system;

namespace raft {
  namespace rpc {
    class tcp_connection {
      public:
        static shared_ptr<tcp_connection> create(tcp_socket socket) {
          return shared_ptr<tcp_connection>(new tcp_connection(std::move(socket)));
        }
        tcp_socket& socket() {
          return socket_;
        }
      private:
        tcp_connection(tcp_socket socket): socket_(move(socket)), message_() {}
        tcp_socket socket_;
        string message_;
    };

    void handle_write(const error_code& error, size_t /*bytes_transferred*/) {
      if (error) {
        cerr << "Error writing to socket: " << error.value() << " - " << error.message() << endl;
      }
    }

    void tcp::server::run() {
      start_accept();
      try {
        for (size_t i = 0; i < THREAD_POOL_SIZE; i++) {
          thread_pool_[i] = shared_ptr<thread>(
              new thread([this](){ io_service_.run();}));
        }
      } catch (exception& e) {
        cerr << e.what() << endl;
      }
    }

    void tcp::server::stop() {
      io_service_.stop();
      for (size_t i = 0; i < THREAD_POOL_SIZE; i++) {
        thread_pool_[i]->join();
      }
    }

    void tcp::server::start_accept(){
      auto conn = tcp_connection::create(move(socket_));
      auto handler = bind(&server::handle_accept, this, conn, _1);
      acceptor_.async_accept(conn->socket(), handler);
    }

    void tcp::server::handle_accept(
        shared_ptr<tcp_connection> conn,
        const error_code& error) {
      if (!error) {
        auto res = append_handler_(append_entries_request());
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
  }
}
