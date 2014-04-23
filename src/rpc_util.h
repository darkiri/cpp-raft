#ifndef RAFT_RPC_UTIL
#define RAFT_RPC_UTIL

#include <iostream>
#include <boost/asio.hpp>
#include "rpc.h"
#include "proto/raft.pb.h"

namespace raft {
  namespace rpc {

    typedef boost::asio::ip::tcp::resolver tcp_resolver;
    typedef boost::asio::ip::tcp::socket tcp_socket;

    void serialize_int(char* data, int n);
    int deserialize_int(std::array<char, tcp::HEADER_LENGTH> data);
    int get_size(const google::protobuf::Message& m);
    std::shared_ptr<char> pack(const google::protobuf::Message& m);

    template<typename Message>
    void write_message(tcp_socket& s, const Message& request, std::function<void(const Message&)> h);
    template<typename Message>
    void read_message(tcp_socket& s, std::function<void(const Message&)> h);

    using namespace std;
    using namespace boost::asio;
    using namespace boost::system;

    template<typename Message>
      void write_message(tcp_socket& s, const Message& request, function<void(const Message&)> h) {
        auto data = pack(request);
        auto handler = [data, request, h] (const error_code& ec, size_t size) {
          if (!ec) {
            cout << "Writing completed: " << size << " bytes" << endl;
            h(request);
          } else {
            cerr << "Error writing to socket: " << ec.value() << " - " << ec.message() << endl;
          }
        };
        cout <<  "Writing: " << get_size(request) << " bytes" << endl;
        async_write(s, buffer(data.get(), get_size(request)), handler);
      }

    template<typename Message>
      void read_message(tcp_socket& s, function<void(const Message&)> h) {
        array<char, tcp::HEADER_LENGTH> data;
        cout <<  "Reading header: " << tcp::HEADER_LENGTH << " bytes" << endl;
        read(s, buffer(&data, tcp::HEADER_LENGTH));
        size_t size = deserialize_int(data);
        auto payload = shared_ptr<char>(new char[size]);
        auto handler = [payload, h] (const error_code& ec, size_t size) {
          if (!ec) {
            cout << "Reading completed: " << size << " bytes" << endl;
            Message message;
            message.ParseFromArray(payload.get(), size);
            h(message);
          } else {
            cerr << "Error reading from socket: " << ec.value() << " - " << ec.message() << endl;
          }
        };
        cout <<  "Reading payload: " << size << " bytes" << endl;
        async_read(s, buffer(payload.get(), size), handler);
      }
  }
}
#endif
