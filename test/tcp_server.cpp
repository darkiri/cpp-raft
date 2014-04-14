#include <iostream>
#include "proto/raft.pb.h"
#include "rpc.h"

using namespace std;
using namespace raft;

append_entries_response test_handler(const append_entries_request&) {
  std::cout<<"append entries handler"<<std::endl;
  append_entries_response res;
  res.set_term(2);
  res.set_success(true);
  return res;
}

int main() {
  config_server c;
  c.set_id(1);
  c.set_port(7576);
  rpc::tcp::server s(c, test_handler);
  s.run();
  char* tmp = 0;
  cin.getline(tmp, 0);
  s.stop();
  return 0;
}
