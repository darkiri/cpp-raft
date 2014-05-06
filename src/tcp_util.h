#ifndef RAFT_RPC_UTIL
#define RAFT_RPC_UTIL

#include <iostream>
#include <boost/asio.hpp>
#include "rpc.h"

namespace raft {
  namespace rpc {

    typedef boost::asio::ip::tcp::socket tcp_socket;

    inline void serialize_int(char* data, int n) {
      data[0] = (n >> 24) & 0xFF;
      data[1] = (n >> 16) & 0xFF;
      data[2] = (n >> 8) & 0xFF;
      data[3] = n & 0xFF;
    }

    inline int deserialize_int(std::array<char, 4> data) {
      auto res = 0;
      for (auto i = 0; i < 4; i++){
        res += res*256 + data[i];
      }
      return res;
    }

    inline int get_size(const google::protobuf::Message& m) {
      return tcp::HEADER_LENGTH + m.ByteSize();
    }

    inline std::shared_ptr<char> pack(const google::protobuf::Message& m) {
      auto message_size = get_size(m);
      auto data = std::shared_ptr<char>(new char[message_size]);
      serialize_int(data.get(), m.ByteSize());
      m.SerializeToArray(data.get() + tcp::HEADER_LENGTH, m.ByteSize());
      return data;
    }

    template<typename Message>
    void write_message(tcp_socket& s, const Message& request, std::function<void(const Message&)> h) {
      auto data = pack(request);
      auto handler = [data, &request, h] (const boost::system::error_code& ec, size_t size) {
        if (!ec) {
          std::cout << "Writing completed: " << size << " bytes" << std::endl;
          h(request);
        } else {
          std::cerr << "Error writing to socket: " << ec.value() << " - " << ec.message() << std::endl;
        }
      };
      std::cout <<  "Writing: " << get_size(request) << " bytes" << std::endl;
      async_write(s, boost::asio::buffer(data.get(), get_size(request)), handler);
    }

    template<typename Message>
    void read_message(tcp_socket& s, std::function<void(const Message&)> h) {
      std::array<char, tcp::HEADER_LENGTH> data;
      std::cout <<  "Reading header: " << tcp::HEADER_LENGTH << " bytes" << std::endl;
      read(s, boost::asio::buffer(&data, tcp::HEADER_LENGTH));
      auto size = deserialize_int(data);
      auto payload = std::shared_ptr<char>(new char[size]);
      auto handler = [payload, h] (const boost::system::error_code& ec, size_t size) {
        if (!ec) {
          std::cout << "Reading completed: " << size << " bytes" << std::endl;
          Message message;
          message.ParseFromArray(payload.get(), size);
          h(message);
        } else {
          std::cerr << "Error reading from socket: " << ec.value() << " - " << ec.message() << std::endl;
        }
      };
      std::cout <<  "Reading payload: " << size << " bytes" << std::endl;
      async_read(s, boost::asio::buffer(payload.get(), size), handler);
    }
  }
}
#endif
