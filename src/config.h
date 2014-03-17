#ifndef RAFT_CONFIG
#define RAFT_CONFIG

#include <vector>
#include <string>

namespace raft {
  struct config {
    struct server {
      std::string ip;
      int id;
    };
    config(std::initializer_list<server> p) : peers(p) {}
    std::vector<server> peers;
  };
}
#endif
