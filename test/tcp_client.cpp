#include <iostream>
#include "proto/raft.pb.h"
#include "rpc.h"

using namespace std;
using namespace raft;

void on_appended(const append_entries_response& r) {
  cout << "Response: term = " << r.term() << ", success = " << r.success() << endl;
}
int main() {
  config_server config;
  config.set_id(1);
  config.set_port(7576);
  timeout t;
  rpc::tcp::client c(config, t);
  append_entries_request r;
  c.append_entries_async(r, on_appended);
  char* tmp = 0;
  cin.getline(tmp, 0);
  return 0;
}
