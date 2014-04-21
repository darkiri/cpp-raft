#include <iostream>

#include "tcp_connection.h"

namespace raft {
  namespace rpc {

    using namespace std;
    using namespace std::placeholders;
    using namespace boost::asio;
    using namespace boost::system;

    void tcp_connection::handle_read(const error_code& error, shared_ptr<char> payload, int size) {
      if (!error) {
        cout << "handle read " << size << " bytes" << endl;
        append_entries_request r;
        r.ParseFromArray(payload.get(), size);
        auto res = handler_(r);

        auto message_size = res.ByteSize() + tcp::HEADER_LENGTH;
        auto data = shared_ptr<char>(new char[message_size]);
        serialize_int(data.get(), res.ByteSize());
        res.SerializeToArray(data.get() + tcp::HEADER_LENGTH, res.ByteSize());
        cout << "sending " << message_size << " bytes" << endl;

        auto self = shared_from_this();
        auto handler = [this, self](const error_code& error, size_t s) {
          self->handle_write(error, s);
        };
        async_write(socket_, buffer(data.get(), message_size), handler);
      } else {
        cerr << "Error reading from socket: " << error.value() << " - " << error.message() << endl;
      }
    }
    void tcp_connection::start(){
      cout << "connection started" << endl;
      array<char, tcp::HEADER_LENGTH> data;
      read(socket_, buffer(&data, tcp::HEADER_LENGTH));
      size_t size = deserialize_int(data);
      cout <<  "payload size: " << size << endl;
      auto self = shared_from_this();
      auto payload = shared_ptr<char>(new char[size]);
      auto handler = [this, self, payload] (const error_code& ec, size_t size) {
        self->handle_read(ec, payload, size);
      };
      async_read(socket_, buffer(payload.get(), size), handler);
    }

    void tcp_connection::handle_write(const error_code& error, size_t /*bytes_transferred*/) {
      if (!error) {
        cout << "handle write" << endl;
        error_code ignored_ec;
        socket_.shutdown(tcp_socket::shutdown_both, ignored_ec);
      } else {
        cerr << "Error writing to socket: " << error.value() << " - " << error.message() << endl;
      }
    }

  }
}
