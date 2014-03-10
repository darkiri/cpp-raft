#include <gtest/gtest.h>
#include "test_funcs.h"

using namespace std;

namespace raft {

  class NodeTest : public ::testing::Test {};

  typedef Node<InMemoryLog> InMemoryNode;

  AppendEntriesArgs MakeAppendArgs(
      unsigned int term,
      unsigned int prev_log_index,
      unsigned int prev_log_term,
      const vector<LogEntry>& entries = vector<LogEntry>(),
      unsigned int commit_index = 0){
    return AppendEntriesArgs {term, prev_log_index, prev_log_term, entries, commit_index};
  }

  RequestVoteArgs MakeRequestVoteArgs(
      unsigned int term,
      unsigned int candidate_id,
      unsigned int last_log_index = 0,
      unsigned int last_log_term = 0){
    return RequestVoteArgs {term, candidate_id, last_log_index, last_log_term};
  }

  template<class Log>
  void ExpectLogSize(Log& log, int s) {
    EXPECT_EQ(s, log.Size());
  }

  template<class Log>
  void ExpectLogTerm(Log& log, int i, int t) {
    EXPECT_EQ(t, log.Begin()[i].term);
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
    log.Append(CreateLogEntry(1));
    log.Append(CreateLogEntry(2));
    InMemoryNode node(log);
    auto args = MakeAppendArgs(1, 0, 0);
    auto res = node.AppendEntries(args);
    EXPECT_EQ(2, res.term);
  }

  TEST_F(NodeTest, AppendEntries_Returns_False_For_Empty_Log) {
    InMemoryLog log;
    InMemoryNode node(log);
    auto args = MakeAppendArgs(1, 0, 0);
    auto res = node.AppendEntries(args);
    EXPECT_FALSE(res.success);
  }

  TEST_F(NodeTest, AppendEntries_Returns_False_If_Term_Is_Lower_As_CurrentTerm) {
    InMemoryLog log;
    log.Append(CreateLogEntry(1));
    log.Append(CreateLogEntry(2));
    log.Append(CreateLogEntry(3));
    InMemoryNode node(log);
    auto args = MakeAppendArgs(1, 0, 0);
    auto res = node.AppendEntries(args);
    EXPECT_FALSE(res.success);
  }

  TEST_F(NodeTest, AppendEntries_Returns_True_If_Term_Is_Same_As_CurrentTerm) {
    InMemoryLog log;
    log.Append(CreateLogEntry(2));
    InMemoryNode node(log);
    auto args = MakeAppendArgs(2, 0, 0);
    auto res = node.AppendEntries(args);
    EXPECT_TRUE(res.success);
  }

  TEST_F(NodeTest, AppendEntries_Returns_False_If_Log_DoesNotContain_prevLogTerm_At_prevLogIndex) {
    InMemoryLog log;
    log.Append(CreateLogEntry(1));
    log.Append(CreateLogEntry(2));
    log.Append(CreateLogEntry(3));
    log.Append(CreateLogEntry(3));
    InMemoryNode node(log);
    auto args = MakeAppendArgs(3, 1, 1);
    auto res = node.AppendEntries(args);
    EXPECT_FALSE(res.success);
  }

  TEST_F(NodeTest, AppendEntries_Returns_False_If_Log_DoesNotContain_prevLogTerm_At_prevLogIndex_withMissingEntries) {
    InMemoryLog log;
    log.Append(CreateLogEntry(1));
    log.Append(CreateLogEntry(2));
    log.Append(CreateLogEntry(3));
    InMemoryNode node(log);
    auto args = MakeAppendArgs(5, 10, 4);
    auto res = node.AppendEntries(args);
    EXPECT_FALSE(res.success);
  }

  TEST_F(NodeTest, AppendEntries_TrimsLog_If_TermDoesNotMatch) {
    InMemoryLog log;
    log.Append(CreateLogEntry(1));
    log.Append(CreateLogEntry(2));
    log.Append(CreateLogEntry(2));
    log.Append(CreateLogEntry(3));
    InMemoryNode node(log);
    auto entries = vector<LogEntry> {CreateLogEntry(4), CreateLogEntry(5)};
    auto args = MakeAppendArgs(5, 1, 2, entries);
    auto res = node.AppendEntries(args);
    EXPECT_TRUE(res.success);
    ExpectLogSize(log, 4);
    ExpectLogTerm(log, 0, 1);
    ExpectLogTerm(log, 1, 2);
    ExpectLogTerm(log, 2, 4);
    ExpectLogTerm(log, 3, 5);
  }

  TEST_F(NodeTest, AppendEntries_AppendsNewEntries) { // yep, this is the name
    InMemoryLog log;
    log.Append(CreateLogEntry(2));
    InMemoryNode node(log);
    auto entries = vector<LogEntry> {CreateLogEntry(4)};
    auto args = MakeAppendArgs(5, 0, 2, entries);
    auto res = node.AppendEntries(args);
    EXPECT_TRUE(res.success);
    ExpectLogSize(log, 2);
    ExpectLogTerm(log, 0, 2);
    ExpectLogTerm(log, 1, 4);
  }

  TEST_F(NodeTest, AppendEntries_KeepAlive_Does_Not_AppendEntries) {
    InMemoryLog log;
    log.Append(CreateLogEntry(1));
    log.Append(CreateLogEntry(2));
    InMemoryNode node(log);
    auto entries = vector<LogEntry>();
    auto args = MakeAppendArgs(2, 1, 2, entries);
    auto res = node.AppendEntries(args);
    EXPECT_TRUE(res.success);
    ExpectLogSize(log, 2);
    ExpectLogTerm(log, 0, 1);
    ExpectLogTerm(log, 1, 2);
  }

  TEST_F(NodeTest, AppendEntries_Updates_CommitIndex) {
    InMemoryLog log;
    log.Append(CreateLogEntry(1));
    log.Append(CreateLogEntry(2));
    log.Append(CreateLogEntry(2));
    InMemoryNode node(log);
    auto entries = vector<LogEntry>();
    auto args = MakeAppendArgs(2, 1, 2, entries, 1);
    auto res = node.AppendEntries(args);
    EXPECT_TRUE(res.success);
    EXPECT_EQ(1, node.GetCommitIndex());
  }

  TEST_F(NodeTest, RequestVote_Returns_False_If_Term_Is_Lower_As_CurrentTerm) {
    InMemoryLog log;
    log.Append(CreateLogEntry(1));
    log.Append(CreateLogEntry(2));
    InMemoryNode node(log);
    auto args = MakeRequestVoteArgs(1, 1, 0, 1);
    auto res = node.RequestVote(args);
    EXPECT_FALSE(res.success);
  }

  TEST_F(NodeTest, RequestVote_Returns_CurrentTerm) {
    InMemoryLog log;
    log.Append(CreateLogEntry(1));
    log.Append(CreateLogEntry(2));
    InMemoryNode node(log);
    auto args = MakeRequestVoteArgs(1, 1, 0, 1);
    auto res = node.RequestVote(args);
    EXPECT_EQ(2, res.term);
  }

  TEST_F(NodeTest, RequestVote_Returns_False_If_Already_VotedFor_Another_Candiate) {
    InMemoryLog log;
    log.Append(CreateLogEntry(2));
    InMemoryNode node(log);
    auto args = MakeRequestVoteArgs(2, 1, 0, 2);
    node.RequestVote(args);
    args = MakeRequestVoteArgs(2, 2, 0, 2);
    auto res = node.RequestVote(args);
    EXPECT_FALSE(res.success);
  }

  TEST_F(NodeTest, RequestVote_Returns_False_If_CandidatesLog_Is_Not_UpToDate) {
    InMemoryLog log;
    log.Append(CreateLogEntry(1));
    log.Append(CreateLogEntry(2));
    log.Append(CreateLogEntry(2));
    InMemoryNode node(log);
    auto args = MakeRequestVoteArgs(2, 1, 0, 1);
    auto res = node.RequestVote(args);
    EXPECT_FALSE(res.success);
  }

  TEST_F(NodeTest, RequestVote_Returns_True_If_Vote_Granted) {
    InMemoryLog log;
    log.Append(CreateLogEntry(2));
    log.Append(CreateLogEntry(2));
    InMemoryNode node(log);
    auto args = MakeRequestVoteArgs(2, 1, 1, 2);
    auto res = node.RequestVote(args);
    EXPECT_TRUE(res.success);
    EXPECT_EQ(1, node.GetVotedFor());
  }

  TEST_F(NodeTest, RequestVote_Returns_True_If_Already_VotedFor_Same_Candiate) {
    InMemoryLog log;
    log.Append(CreateLogEntry(2));
    InMemoryNode node(log);
    auto args = MakeRequestVoteArgs(2, 1, 0, 2);
    node.RequestVote(args);
    args = MakeRequestVoteArgs(2, 1, 0, 2);
    auto res = node.RequestVote(args);
    EXPECT_TRUE(res.success);
  }

  TEST_F(NodeTest, StartElection_IncrementCurrentTerm){
    InMemoryLog log;
    log.Append(CreateLogEntry(2));
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
    log.Append(CreateLogEntry(2));
    InMemoryNode node(log);
    node.StartElection();
    auto args = MakeAppendArgs(4, 0, 2);
    auto res = node.AppendEntries(args);
    EXPECT_TRUE(res.success);
    EXPECT_EQ(NodeState::FOLLOWER, node.GetState());
  }

  TEST_F(NodeTest, KeepAlive_FromNewLeader_UpdateCurrentTerm) {
    InMemoryLog log;
    log.Append(CreateLogEntry(2));
    InMemoryNode node(log);
    auto args = MakeAppendArgs(3, 0, 2);
    auto res = node.AppendEntries(args);
    EXPECT_TRUE(res.success);
    EXPECT_EQ(3, node.GetCurrentTerm());
  }

  TEST_F(NodeTest, RequestVote_FromNewLeader_UpdateCurrentTerm) {
    InMemoryLog log;
    log.Append(CreateLogEntry(2));
    InMemoryNode node(log);
    auto args = MakeRequestVoteArgs(3, 0, 0, 0);
    node.RequestVote(args);
    EXPECT_EQ(3, node.GetCurrentTerm());
  }

  TEST_F(NodeTest, RequestVote_FromNewLeader_ConvertToFollower) {
    InMemoryLog log;
    log.Append(CreateLogEntry(2));
    InMemoryNode node(log);
    node.StartElection();
    auto args = MakeRequestVoteArgs(4, 0, 0, 0);
    node.RequestVote(args);
    EXPECT_EQ(NodeState::FOLLOWER, node.GetState());
  }
}
