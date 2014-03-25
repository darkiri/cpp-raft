#include <iostream>
#include "rpc.h"
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;
namespace raft {
  namespace rpc {
    namespace tcp1 {
      void server::run() {
        try {

          start_accept();
          this->io_service_.run();
        }
        catch (std::exception& e) {
          std::cerr << e.what() << std::endl;
        }
      }

      void server::start_accept(){
        this->acceptor_.async_accept(this->socket_, boost::bind(&server::handle_accept, this, boost::asio::placeholders::error));
      }

      void server::handle_accept(const boost::system::error_code& error) {
        if (!error) {
          append_handler_(append_entries_request());
          boost::asio::async_write(this->socket_, boost::asio::buffer("Hello, World!\r\n"),
              [](const boost::system::error_code&, size_t){});
          start_accept();
        } else {
          std::cout<<"error: " << error <<std::endl;
        }
      }
      void server::handle_write(const boost::system::error_code&, size_t){
      }
    }
  }
}
