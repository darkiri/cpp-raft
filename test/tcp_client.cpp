#include <iostream>
#include "proto/raft.pb.h"
#include "rpc.h"

using namespace std;
using namespace raft;

void on_appended(const append_entries_response& r) {
  cout << "Response: term = " << r.term() << ", success = " << r.success() << endl;
}

void on_timeout() {
  cout << "timeout in append entries "<< endl;
}

int main() {
  config_server config;
  config.set_id(1);
  config.set_port(7576);
  timeout t;
  rpc::tcp::client c(config, t);
  append_entries_request r;
  r.set_term(223);
  c.append_entries_async(r, on_appended, on_timeout);
  char* tmp = 0;
  cin.getline(tmp, 0);
  return 0;
}
