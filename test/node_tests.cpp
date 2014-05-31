#include <gtest/gtest.h>
#include "test_funcs.h"

using namespace std;

namespace raft {

  class NodeTest : public ::testing::Test {};

  typedef Node<InMemoryLog> InMemoryNode;

  append_entries_request make_append_args(
      unsigned int term,
      unsigned int prev_log_index,
      unsigned int prev_log_term,
      unsigned int commit_index = 0){
    append_entries_request request;
    request.set_term(term);
    request.set_prev_log_index(prev_log_index);
    request.set_prev_log_term(prev_log_term);
    request.set_leader_commit(commit_index);
    return request;
  }

  vote_request MakeRequestVoteArgs(
      unsigned int term,
      unsigned int candidate_id,
      unsigned int last_log_index = 0,
      unsigned int last_log_term = 0){
    vote_request request;
    request.set_term(term);
    request.set_candidate_id(candidate_id);
    request.set_last_log_index(last_log_index);
    request.set_last_log_term(last_log_term);
    return request;
  }

  template<class Log>
    void ExpectLogSize(Log& log, int s) {
      EXPECT_EQ(s, log.Size());
    }

  template<class Log>
    void ExpectLogTerm(Log& log, int i, int t) {
      EXPECT_EQ(t, log.Begin()[i].term());
    }

  TEST_F(NodeTest, New_Node_Is_Follower) {
    InMemoryLog log;
    InMemoryNode node(log);
    EXPECT_EQ(NodeState::FOLLOWER, node.GetState());
  }

  TEST_F(NodeTest, New_Node_Commit_Index_0) {
    InMemoryLog log;
    InMemoryNode node(log);
    EXPECT_EQ(0, node.GetCommitIndex());
  }

  TEST_F(NodeTest, AppendEntries_Returns_CurrentTerm) {
    InMemoryLog log;
    log.Append(create_log_entry(1));
    log.Append(create_log_entry(2));
    InMemoryNode node(log);
    auto args = make_append_args(1, 0, 0);
    auto res = node.AppendEntries(args);
    EXPECT_EQ(2, res.term());
  }

  TEST_F(NodeTest, AppendEntries_Returns_False_For_Empty_Log) {
    InMemoryLog log;
    InMemoryNode node(log);
    auto args = make_append_args(1, 0, 0);
    auto res = node.AppendEntries(args);
    EXPECT_FALSE(res.success());
  }

  TEST_F(NodeTest, AppendEntries_Returns_False_If_Term_Is_Lower_As_CurrentTerm) {
    InMemoryLog log;
    log.Append(create_log_entry(1));
    log.Append(create_log_entry(2));
    log.Append(create_log_entry(3));
    InMemoryNode node(log);
    auto args = make_append_args(1, 0, 0);
    auto res = node.AppendEntries(args);
    EXPECT_FALSE(res.success());
  }

  TEST_F(NodeTest, AppendEntries_Returns_True_If_Term_Is_Same_As_CurrentTerm) {
    InMemoryLog log;
    log.Append(create_log_entry(2));
    InMemoryNode node(log);
    auto args = make_append_args(2, 0, 0);
    auto res = node.AppendEntries(args);
    EXPECT_TRUE(res.success());
  }

  TEST_F(NodeTest, AppendEntries_Returns_False_If_Log_DoesNotContain_prevLogTerm_At_prevLogIndex) {
    InMemoryLog log;
    log.Append(create_log_entry(1));
    log.Append(create_log_entry(2));
    log.Append(create_log_entry(3));
    log.Append(create_log_entry(3));
    InMemoryNode node(log);
    auto args = make_append_args(3, 1, 1);
    auto res = node.AppendEntries(args);
    EXPECT_FALSE(res.success());
  }

  TEST_F(NodeTest, AppendEntries_Returns_False_If_Log_DoesNotContain_prevLogTerm_At_prevLogIndex_withMissingEntries) {
    InMemoryLog log;
    log.Append(create_log_entry(1));
    log.Append(create_log_entry(2));
    log.Append(create_log_entry(3));
    InMemoryNode node(log);
    auto args = make_append_args(5, 10, 4);
    auto res = node.AppendEntries(args);
    EXPECT_FALSE(res.success());
  }

  TEST_F(NodeTest, AppendEntries_TrimsLog_If_TermDoesNotMatch) {
    InMemoryLog log;
    log.Append(create_log_entry(1));
    log.Append(create_log_entry(2));
    log.Append(create_log_entry(2));
    log.Append(create_log_entry(3));
    InMemoryNode node(log);
    auto args = make_append_args(5, 1, 2);
    auto e = args.add_entries();
    init_log_entry(e, 4);
    e = args.add_entries();
    init_log_entry(e, 5);
    auto res = node.AppendEntries(args);
    EXPECT_TRUE(res.success());
    ExpectLogSize(log, 4);
    ExpectLogTerm(log, 0, 1);
    ExpectLogTerm(log, 1, 2);
    ExpectLogTerm(log, 2, 4);
    ExpectLogTerm(log, 3, 5);
  }

  TEST_F(NodeTest, AppendEntries_AppendsNewEntries) { // yep, this is the name
    InMemoryLog log;
    log.Append(create_log_entry(2));
    InMemoryNode node(log);
    auto args = make_append_args(5, 0, 2);
    auto e = args.add_entries();
    init_log_entry(e, 4);
    auto res = node.AppendEntries(args);
    EXPECT_TRUE(res.success());
    ExpectLogSize(log, 2);
    ExpectLogTerm(log, 0, 2);
    ExpectLogTerm(log, 1, 4);
  }

  TEST_F(NodeTest, AppendEntries_KeepAlive_Does_Not_AppendEntries) {
    InMemoryLog log;
    log.Append(create_log_entry(1));
    log.Append(create_log_entry(2));
    InMemoryNode node(log);
    auto args = make_append_args(2, 1, 2);
    auto res = node.AppendEntries(args);
    EXPECT_TRUE(res.success());
    ExpectLogSize(log, 2);
    ExpectLogTerm(log, 0, 1);
    ExpectLogTerm(log, 1, 2);
  }

  TEST_F(NodeTest, AppendEntries_Updates_CommitIndex) {
    InMemoryLog log;
    log.Append(create_log_entry(1));
    log.Append(create_log_entry(2));
    log.Append(create_log_entry(2));
    InMemoryNode node(log);
    auto args = make_append_args(2, 1, 2, 1);
    auto res = node.AppendEntries(args);
    EXPECT_TRUE(res.success());
    EXPECT_EQ(1, node.GetCommitIndex());
  }

  TEST_F(NodeTest, RequestVote_Returns_False_If_Term_Is_Lower_As_CurrentTerm) {
    InMemoryLog log;
    log.Append(create_log_entry(1));
    log.Append(create_log_entry(2));
    InMemoryNode node(log);
    auto args = MakeRequestVoteArgs(1, 1, 0, 1);
    auto res = node.RequestVote(args);
    EXPECT_FALSE(res.granted());
  }

  TEST_F(NodeTest, RequestVote_Returns_CurrentTerm) {
    InMemoryLog log;
    log.Append(create_log_entry(1));
    log.Append(create_log_entry(2));
    InMemoryNode node(log);
    auto args = MakeRequestVoteArgs(1, 1, 0, 1);
    auto res = node.RequestVote(args);
    EXPECT_EQ(2, res.term());
  }

  TEST_F(NodeTest, RequestVote_Returns_False_If_Already_VotedFor_Another_Candiate) {
    InMemoryLog log;
    log.Append(create_log_entry(2));
    InMemoryNode node(log);
    auto args = MakeRequestVoteArgs(2, 1, 0, 2);
    node.RequestVote(args);
    args = MakeRequestVoteArgs(2, 2, 0, 2);
    auto res = node.RequestVote(args);
    EXPECT_FALSE(res.granted());
  }

  TEST_F(NodeTest, RequestVote_Returns_False_If_CandidatesLog_Is_Not_UpToDate) {
    InMemoryLog log;
    log.Append(create_log_entry(1));
    log.Append(create_log_entry(2));
    log.Append(create_log_entry(2));
    InMemoryNode node(log);
    auto args = MakeRequestVoteArgs(2, 1, 0, 1);
    auto res = node.RequestVote(args);
    EXPECT_FALSE(res.granted());
  }

  TEST_F(NodeTest, RequestVote_Returns_True_If_Vote_Granted) {
    InMemoryLog log;
    log.Append(create_log_entry(2));
    log.Append(create_log_entry(2));
    InMemoryNode node(log);
    auto args = MakeRequestVoteArgs(2, 1, 1, 2);
    auto res = node.RequestVote(args);
    EXPECT_TRUE(res.granted());
    EXPECT_EQ(1, node.GetVotedFor());
  }

  TEST_F(NodeTest, RequestVote_Returns_True_If_Already_VotedFor_Same_Candiate) {
    InMemoryLog log;
    log.Append(create_log_entry(2));
    InMemoryNode node(log);
    auto args = MakeRequestVoteArgs(2, 1, 0, 2);
    node.RequestVote(args);
    args = MakeRequestVoteArgs(2, 1, 0, 2);
    auto res = node.RequestVote(args);
    EXPECT_TRUE(res.granted());
  }

  TEST_F(NodeTest, StartElection_IncrementCurrentTerm){
    InMemoryLog log;
    log.Append(create_log_entry(2));
    InMemoryNode node(log);
    node.StartElection();
    EXPECT_EQ(3, node.GetCurrentTerm());
  }

  TEST_F(NodeTest, StartElection_ConvertsToCandidate){
    InMemoryLog log;
    InMemoryNode node(log);
    node.StartElection();
    EXPECT_EQ(NodeState::CANDIDATE, node.GetState());
  }

  TEST_F(NodeTest, AppendEntries_FromNewLeader_ConvertToFollower) {
    InMemoryLog log;
    log.Append(create_log_entry(2));
    InMemoryNode node(log);
    node.StartElection();
    auto args = make_append_args(4, 0, 2);
    auto res = node.AppendEntries(args);
    EXPECT_TRUE(res.success());
    EXPECT_EQ(NodeState::FOLLOWER, node.GetState());
  }

  TEST_F(NodeTest, KeepAlive_FromNewLeader_UpdateCurrentTerm) {
    InMemoryLog log;
    log.Append(create_log_entry(2));
    InMemoryNode node(log);
    auto args = make_append_args(3, 0, 2);
    auto res = node.AppendEntries(args);
    EXPECT_TRUE(res.success());
    EXPECT_EQ(3, node.GetCurrentTerm());
  }

  TEST_F(NodeTest, RequestVote_FromNewLeader_UpdateCurrentTerm) {
    InMemoryLog log;
    log.Append(create_log_entry(2));
    InMemoryNode node(log);
    auto args = MakeRequestVoteArgs(3, 0, 0, 0);
    node.RequestVote(args);
    EXPECT_EQ(3, node.GetCurrentTerm());
  }

  TEST_F(NodeTest, RequestVote_FromNewLeader_ConvertToFollower) {
    InMemoryLog log;
    log.Append(create_log_entry(2));
    InMemoryNode node(log);
    node.StartElection();
    auto args = MakeRequestVoteArgs(4, 0, 0, 0);
    node.RequestVote(args);
    EXPECT_EQ(NodeState::FOLLOWER, node.GetState());
  }
}
