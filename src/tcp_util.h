#ifndef RAFT_RPC_UTIL
#define RAFT_RPC_UTIL

#include <iostream>
#include <boost/asio.hpp>
#include "rpc.h"
#include "logging.h"

namespace raft {
  namespace rpc {

    typedef boost::asio::ip::tcp::socket tcp_socket;

    const static int TCP_HEADER_LENGTH = 4;

    inline void serialize_int(char* data, int n) {
      data[0] = (n >> 24) & 0xFF;
      data[1] = (n >> 16) & 0xFF;
      data[2] = (n >> 8) & 0xFF;
      data[3] = n & 0xFF;
    }

    inline int deserialize_int(char data[4]) {
      auto res = 0;
      for (auto i = 0; i < 4; i++){
        res += res*256 + data[i];
      }
      return res;
    }

    inline int get_size(const google::protobuf::Message& m) {
      return TCP_HEADER_LENGTH + m.ByteSize();
    }

    inline std::shared_ptr<char> pack(const google::protobuf::Message& m) {
      auto message_size = get_size(m);
      auto data = std::shared_ptr<char>(new char[message_size]);
      serialize_int(data.get(), m.ByteSize());
      m.SerializeToArray(data.get() + TCP_HEADER_LENGTH, m.ByteSize());
      return data;
    }

    template<typename Message>
    void write_message(tcp_socket& s, const Message& request, std::function<void()> h, error_handler eh) {
      auto data = pack(request);
      auto handler = [data, h, eh] (const boost::system::error_code& ec, size_t size) {
        if (!ec) {
          LOG_TRACE << "Writing completed: " << size << " bytes";
          h();
        } else {
          LOG_ERROR << "Error writing to socket: " << ec.value() << " - " << ec.message();
          eh();
        }
      };
      LOG_TRACE <<  "Writing: " << get_size(request) << " bytes";
      async_write(s, boost::asio::buffer(data.get(), get_size(request)), handler);
    }

    template<typename Message>
    void read_message(tcp_socket& s, std::function<void(const Message&)> h, error_handler eh) {
      auto header = std::shared_ptr<char>(new char[TCP_HEADER_LENGTH]);

      auto header_read_handler = [&s, header, h, eh](const boost::system::error_code& ec1, size_t size) {
        if (!ec1) {
          LOG_TRACE << "Header reading completed: " << size << " bytes";
          auto size = deserialize_int(header.get());
          auto payload = std::shared_ptr<char>(new char[size]);
          auto handler = [payload, h, eh] (const boost::system::error_code& ec, size_t size) {
            if (!ec) {
              LOG_TRACE << "Reading completed: " << size << " bytes";
              Message message;
              message.ParseFromArray(payload.get(), size);
              h(message);
            } else {
              LOG_ERROR << "Error reading from socket: " << ec.value() << " - " << ec.message();
              eh();
            }
          };
          LOG_TRACE <<  "Ready to read payload: " << size << " bytes";
          async_read(s, boost::asio::buffer(payload.get(), size), handler);
        } else {
          LOG_ERROR << "Error reading from socket: " << ec1.value() << " - " << ec1.message();
          eh();
        };
      };

      LOG_TRACE <<  "Ready to read header: " << TCP_HEADER_LENGTH << " bytes";
      async_read(s, boost::asio::buffer(header.get(), TCP_HEADER_LENGTH), header_read_handler);
    }

    inline std::shared_ptr<boost::asio::deadline_timer> create_deadline(boost::asio::io_service& ios, timeout t, error_handler h) {
      auto timer = std::shared_ptr<boost::asio::deadline_timer>(new boost::asio::deadline_timer(ios));
      timer->expires_from_now(boost::posix_time::milliseconds(t.get()));
      timer->async_wait([timer, h] (const boost::system::error_code& ec) {
        if (!ec) {
          h();
        } else if (ec != boost::asio::error::operation_aborted) {
          LOG_ERROR << "Error waiting on socket: " << ec.value() << " - " << ec.message();
        }
      });
      return timer;
    }

    inline void extend_deadline(std::shared_ptr<boost::asio::deadline_timer> timer, timeout t, error_handler h) {
      if (timer->expires_from_now(boost::posix_time::milliseconds(t.get()))) {
        timer->async_wait([timer, h] (const boost::system::error_code& ec) {
          if (!ec) {
            h();
          } else if (ec != boost::asio::error::operation_aborted) {
            LOG_ERROR << "Error waiting on timer: " << ec.value() << " - " << ec.message();
          }
        });
      } else {
          LOG_ERROR << "Timeout already expired";
      }
    }
  }
}
#endif
