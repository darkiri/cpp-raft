#include <iostream>
#include "proto/raft.pb.h"
#include "rpc.h"
#include "logging.h"
#include <thread>

using namespace std;
using namespace raft;

void on_appended(const append_entries_response& r) {
  LOG_INFO << "Append entries response: term = " << r.term() << ", success = " << r.success();
}

void on_voted(const vote_response& r) {
  LOG_INFO << "Vote response: term = " << r.term() << ", granted = " << r.granted();
}

void on_timeout() {
  LOG_INFO << "An error occured";
}

int main() {
  init_log();
  config_server config;
  config.set_id(1);
  config.set_port(7576);
  timeout t;
  rpc::tcp::client c(config, t);
  unique_ptr<append_entries_request> r(new append_entries_request());
  r->set_term(223);
  r->set_leader_id(2);
  r->set_prev_log_index(222);
  r->set_prev_log_term(333);
  r->set_leader_commit(131);
  c.append_entries_async(move(r), on_appended, on_timeout);

  this_thread::sleep_for(chrono::milliseconds(500));
  unique_ptr<vote_request> r2(new vote_request());
  r2->set_term(223);
  r2->set_candidate_id(2);
  r2->set_last_log_term(222);
  r2->set_last_log_index(333);
  c.request_vote_async(move(r2), on_voted, on_timeout);

  char* tmp = 0;
  cin.getline(tmp, 0);
  return 0;
}
