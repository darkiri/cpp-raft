#include <boost/asio.hpp>

#include "rpc.h"

namespace raft {
  namespace rpc {

    using namespace boost::asio;

    struct tcp::client::impl {
      public:
        impl(const config_server& c, const timeout& t) :
          config_(c),
          timeout_(t),
          ios_()
      { }
        void append_entries_async(const append_entries_request&, on_appended_handler);

      private:
        const config_server& config_;
        const timeout& timeout_;

        io_service ios_;
    };

    void tcp::client::impl::append_entries_async(const append_entries_request&, on_appended_handler) {
    }

    tcp::client::client(const config_server& c, const timeout& t) :
      pimpl_(new tcp::client::impl(c, t)) {} 

    void tcp::client::append_entries_async(const append_entries_request& r, on_appended_handler h) {
      pimpl_->append_entries_async(r, h);
    }

    tcp::client::~client() = default;
  }
}
