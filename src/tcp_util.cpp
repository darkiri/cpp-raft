#include <system_error>
#include "tcp_util.h"
#include "logging.h"

using namespace raft::rpc;
using namespace boost::system;

void serialize_int(char* data, int n) {
  data[0] = (n >> 24) & 0xFF;
  data[1] = (n >> 16) & 0xFF;
  data[2] = (n >> 8) & 0xFF;
  data[3] = n & 0xFF;
}

int deserialize_int(char data[4]) {
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

void raft::rpc::write_message_async(tcp_socket& s, const raft_message& request, std::function<void()> h, error_handler eh) {
  auto data = pack(request);
  auto handler = [data, h, eh] (const error_code& ec, size_t size) {
    if (!ec) {
      LOG_TRACE << "Writing completed: " << size << " bytes";
      h();
    } else {
      eh(ec);
    }
  };
  LOG_TRACE << "Writing: " << get_size(request) << " bytes";
  async_write(s, boost::asio::buffer(data.get(), get_size(request)), handler);
}

void raft::rpc::read_message_async(tcp_socket& s, std::function<void(const raft_message&)> h, error_handler eh) {
  auto header = std::shared_ptr<char>(new char[TCP_HEADER_LENGTH]);

  auto header_read_handler = [&s, header, h, eh](const error_code& ec, size_t) {
    if (!ec) {
      auto size = deserialize_int(header.get());
      auto payload = std::shared_ptr<char>(new char[size]);

      LOG_TRACE << "Ready to read payload: " << size << " bytes";
      //TODO: async_read
      read(s, boost::asio::buffer(payload.get(), size), boost::asio::transfer_exactly(size));

      LOG_TRACE << "Reading completed: " << size << " bytes payload";
      raft_message message;
      message.ParseFromArray(payload.get(), size);
      h(message);
    } else {
      eh(ec);
    };
  };

  LOG_TRACE << "Ready to read header: " << TCP_HEADER_LENGTH << " bytes";
  async_read(s, boost::asio::buffer(header.get(), TCP_HEADER_LENGTH), boost::asio::transfer_exactly(TCP_HEADER_LENGTH), header_read_handler);
}
/*
std::shared_ptr<boost::asio::deadline_timer> raft::rpc::create_deadline(boost::asio::io_service& ios, timeout t, error_handler h) {
  auto timer = std::shared_ptr<boost::asio::deadline_timer>(new boost::asio::deadline_timer(ios));
  timer->expires_from_now(boost::posix_time::milliseconds(t.get()));
  timer->async_wait([timer, h] (const error_code& ec) {
      if (!ec) {
      h(ec);
      } else if (ec != boost::asio::error::operation_aborted) {
      LOG_ERROR << "Error waiting on socket: " << ec.value() << " - " << ec.message();
      }});
  return timer;
}

void raft::rpc::extend_deadline(std::shared_ptr<boost::asio::deadline_timer> timer, timeout t, error_handler h) {
  if (timer->expires_from_now(boost::posix_time::milliseconds(t.get()))) {
    timer->async_wait([timer, h] (const error_code& ec) {
        if (!ec) {
        h(ec);
        } else if (ec != boost::asio::error::operation_aborted) {
        LOG_ERROR << "Error waiting on timer: " << ec.value() << " - " << ec.message();
        }});
  } else {
    LOG_ERROR << "Timeout already expired";
  }
}
*/
