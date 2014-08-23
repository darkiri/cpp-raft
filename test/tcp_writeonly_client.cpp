#include "tcp_writeonly_client.h"
#include "logging.h"

namespace raft {
  namespace rpc {

    using namespace boost::asio;
    using namespace boost::system;

    typedef boost::asio::ip::tcp::resolver tcp_resolver;

    tcp_writeonly_client::tcp_writeonly_client(const config_server& c) :
      config_(c),
      ios_(),
      work_(ios_),
      socket_(ios_),
      thread_(){ 
        tcp_resolver resolver(ios_);
        tcp_resolver::query query(config_.host(), std::to_string(config_.port()));
        auto iterator = resolver.resolve(query);
        connect(socket_, iterator);
        thread_ = std::shared_ptr<std::thread>(new std::thread([this](){ ios_.run();}));
      }

    tcp_writeonly_client::~tcp_writeonly_client() {
      ios_.stop();
      thread_->join();
      LOG_INFO << "tcp_writeonly_client stopped.";
    };

    void tcp_writeonly_client::write(append_entries_request& r) {
      raft_message m;
      m.set_discriminator(raft_message::APPEND_ENTRIES);
      m.set_allocated_append_entries_request(&r);
      write_message_async(socket_, m, [](){}, [](){});
    }
  }
}
