#include <iostream>
#include "proto/raft.pb.h"
#include "rpc.h"
#include "logging.h"

using namespace std;
using namespace raft;

unique_ptr<append_entries_response> test_handler(const append_entries_request& r) {
  LOG_INFO << "Append entries handler";
  LOG_INFO << "Request: term=" << r.term();
  unique_ptr<append_entries_response> res(new append_entries_response());
  res->set_term(2);
  res->set_success(true);
  return res;
}
unique_ptr<vote_response> vote_handler(const vote_request& r) {
  LOG_INFO << "Request vote handler";
  LOG_INFO << "Request: term=" << r.term();
  unique_ptr<vote_response> res(new vote_response());
  res->set_term(2);
  res->set_granted(true);
  return res;
}
void on_timeout() {
  LOG_WARN << "Timeout in Server.";
}

int main() {
  init_log();
  timeout t;
  config_server c;
  c.set_id(1);
  c.set_port(7576);
  rpc::tcp::server s(c, t, test_handler, vote_handler, on_timeout);
  s.run();
  char* tmp = 0;
  cin.getline(tmp, 0);
  s.stop();
  return 0;
}
