#ifndef TCP_WRITEONLY_CLIENT 
#define TCP_WRITEONLY_CLIENT 

#include <memory>
#include <thread>
#include <boost/asio.hpp>

#include "tcp_util.h"

namespace raft {
  namespace rpc {
    class tcp_writeonly_client {
      public:
        tcp_writeonly_client(const config_server& c);
        ~tcp_writeonly_client();

        void write(const append_entries_request&);
      private:
        const config_server& config_;

        boost::asio::io_service ios_;
        boost::asio::io_service::work work_;
        tcp_socket socket_;
        std::shared_ptr<std::thread> thread_;
    };
  }
}
#endif
