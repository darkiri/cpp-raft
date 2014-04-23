#include <memory>
#include "rpc_util.h"

namespace raft {
  namespace rpc {

    using namespace std;
    using namespace boost::asio;
    using namespace boost::system;

    void serialize_int(char* data, int n) {
      data[0] = (n >> 24) & 0xFF;
      data[1] = (n >> 16) & 0xFF;
      data[2] = (n >> 8) & 0xFF;
      data[3] = n & 0xFF;
    }

    int deserialize_int(std::array<char, 4> data) {
      auto res = 0;
      for (auto i = 0; i < 4; i++){
        res += res*256 + data[i];
      }
      return res;
    }

    inline int get_size(const google::protobuf::Message& m) {
      return tcp::HEADER_LENGTH + m.ByteSize();
    }

    std::shared_ptr<char> pack(const google::protobuf::Message& m) {
      auto message_size = get_size(m);
      auto data = std::shared_ptr<char>(new char[message_size]);
      serialize_int(data.get(), m.ByteSize());
      m.SerializeToArray(data.get() + tcp::HEADER_LENGTH, m.ByteSize());
      return data;
    }
  }
}
