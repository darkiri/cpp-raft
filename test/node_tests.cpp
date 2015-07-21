#include <gtest/gtest.h>
#include "test_funcs.h"

using namespace std;

namespace raft {

  class NodeTest : public ::testing::Test {};

  typedef node<in_memory_log> InMemoryNode;

  template<class Log>
    void ExpectLogSize(Log& log, int s) {
      EXPECT_EQ(s, log.size());
    }

  template<class Log>
    void ExpectLogTerm(Log& log, int i, int t) {
      EXPECT_EQ(t, log.begin()[i].term());
    }

  TEST_F(NodeTest, New_Node_Is_Follower) {
    in_memory_log log;
    InMemoryNode node(log);

    EXPECT_EQ(node_state::FOLLOWER, node.state());
  }

  TEST_F(NodeTest, New_Node_Commit_Index_0) {
    in_memory_log log;
    InMemoryNode node(log);

    EXPECT_EQ(0, node.commit_index());
  }

  TEST_F(NodeTest, AppendEntries_Returns_CurrentTerm) {
    in_memory_log log;
    log.append(create_log_entry(1));
    log.append(create_log_entry(3));
    log.set_current_term(3);
    InMemoryNode node(log);
    auto args = append_args(2).prev_index_term(1, 2).get();

    auto res = node.append_entries(args);

    EXPECT_EQ(3, res.term());
  }

  TEST_F(NodeTest, AppendEntries_Returns_UpdatedCurrentTerm) {
    in_memory_log log;
    log.append(create_log_entry(1));
    log.append(create_log_entry(2));
    log.set_current_term(2);
    InMemoryNode node(log);
    auto args = append_args(3).prev_index_term(1, 1).get();

    auto res = node.append_entries(args);

    EXPECT_EQ(3, res.term());
  }

  TEST_F(NodeTest, AppendEntries_Returns_True_For_Empty_Log) {
    in_memory_log log;
    InMemoryNode node(log);
    auto args = append_args(1).get();

    auto res = node.append_entries(args);

    EXPECT_TRUE(res.success());
  }

  TEST_F(NodeTest, AppendEntries_Returns_False_If_Term_Is_Lower_As_CurrentTerm) {
    in_memory_log log;
    log.append(create_log_entry(1));
    log.append(create_log_entry(2));
    log.set_current_term(3);
    InMemoryNode node(log);
    auto args = append_args(2).prev_index_term(1, 1).get();

    auto res = node.append_entries(args);

    EXPECT_FALSE(res.success());
  }

  TEST_F(NodeTest, AppendEntries_Returns_True_If_Term_Is_Same_As_CurrentTerm) {
    in_memory_log log;
    log.append(create_log_entry(1));
    log.set_current_term(2);
    InMemoryNode node(log);
    auto args = append_args(2).prev_index_term(1, 1).get();

    auto res = node.append_entries(args);

    EXPECT_TRUE(res.success());
  }

  TEST_F(NodeTest, AppendEntries_Returns_False_If_Log_Not_Matching) {
    in_memory_log log;
    log.append(create_log_entry(1));
    log.append(create_log_entry(2));
    log.append(create_log_entry(3));
    log.append(create_log_entry(3));
    log.set_current_term(3);
    InMemoryNode node(log);
    auto args = append_args(3).prev_index_term(2, 1).get();

    auto res = node.append_entries(args);

    EXPECT_FALSE(res.success());
  }

  TEST_F(NodeTest, AppendEntries_Returns_False_If_Log_Not_Matching_WithMissingEntries) {
    in_memory_log log;
    log.append(create_log_entry(1));
    log.append(create_log_entry(2));
    log.append(create_log_entry(3));
    log.set_current_term(3);
    InMemoryNode node(log);
    auto args = append_args(5).prev_index_term(10, 4).get();

    auto res = node.append_entries(args);

    EXPECT_FALSE(res.success());
  }

  TEST_F(NodeTest, AppendEntries_AppendsNewEntries) { // yep, this is the name
    in_memory_log log;
    log.append(create_log_entry(2));
    log.set_current_term(2);
    InMemoryNode node(log);
    auto args = append_args(5).prev_index_term(1, 2).log_entry(4).get();

    auto res = node.append_entries(args);

    EXPECT_TRUE(res.success());
    ExpectLogSize(log, 3);
    ExpectLogTerm(log, 1, 2);
    ExpectLogTerm(log, 2, 4);
  }

  TEST_F(NodeTest, AppendEntries_TrimsLog_If_TermDoesNotMatch) {
    in_memory_log log;
    log.append(create_log_entry(1));
    log.append(create_log_entry(2));
    log.append(create_log_entry(2));
    log.append(create_log_entry(3));
    log.set_current_term(3);
    InMemoryNode node(log);
    auto args = append_args(5).prev_index_term(2, 2).log_entry(4).log_entry(5).get();

    auto res = node.append_entries(args);

    EXPECT_TRUE(res.success());
    ExpectLogSize(log, 5);
    ExpectLogTerm(log, 1, 1);
    ExpectLogTerm(log, 2, 2);
    ExpectLogTerm(log, 3, 4);
    ExpectLogTerm(log, 4, 5);
  }

  TEST_F(NodeTest, AppendEntries_KeepAlive_Does_Not_AppendEntries) {
    in_memory_log log;
    log.append(create_log_entry(1));
    log.append(create_log_entry(2));
    log.set_current_term(2);
    InMemoryNode node(log);
    auto args = append_args(2).prev_index_term(2, 2).get();

    auto res = node.append_entries(args);

    EXPECT_TRUE(res.success());
    ExpectLogSize(log, 3);
    ExpectLogTerm(log, 1, 1);
    ExpectLogTerm(log, 2, 2);
  }

  TEST_F(NodeTest, AppendEntries_Updates_CommitIndex) {
    in_memory_log log;
    log.append(create_log_entry(1));
    log.append(create_log_entry(2));
    log.append(create_log_entry(2));
    log.set_current_term(2);
    InMemoryNode node(log);
    auto args = append_args(2).prev_index_term(2, 2).commit_index(1).get();

    auto res = node.append_entries(args);

    EXPECT_TRUE(res.success());
    EXPECT_EQ(1, node.commit_index());
  }

  TEST_F(NodeTest, RequestVote_Returns_False_If_Term_Is_Lower_As_CurrentTerm) {
    in_memory_log log;
    log.append(create_log_entry(1));
    log.append(create_log_entry(2));
    InMemoryNode node(log);
    auto args = vote_args(1).candidate(1).last_index_term(2, 1).get();

    auto res = node.request_vote(args);

    EXPECT_FALSE(res.granted());
  }

  TEST_F(NodeTest, RequestVote_Returns_CurrentTerm) {
    in_memory_log log;
    log.append(create_log_entry(1));
    log.append(create_log_entry(2));
    log.set_current_term(3);
    InMemoryNode node(log);
    auto args = vote_args(1).candidate(1).last_index_term(1, 1).get();

    auto res = node.request_vote(args);

    EXPECT_EQ(3, res.term());
  }

  TEST_F(NodeTest, RequestVote_Returns_False_If_Already_VotedFor_Another_Candiate) {
    in_memory_log log;
    log.append(create_log_entry(2));
    log.set_current_term(2);
    InMemoryNode node(log);
    auto args = vote_args(2).candidate(1).last_index_term(1, 2).get();
    node.request_vote(args);
    args = vote_args(2).candidate(2).last_index_term(1, 2).get();

    auto res = node.request_vote(args);

    EXPECT_FALSE(res.granted());
  }

  TEST_F(NodeTest, RequestVote_Returns_False_If_CandidatesLog_Is_Not_UpToDate) {
    in_memory_log log;
    log.append(create_log_entry(1));
    log.append(create_log_entry(2));
    log.append(create_log_entry(2));
    log.set_current_term(2);
    InMemoryNode node(log);
    auto args = vote_args(2).candidate(1).last_index_term(1, 1).get();

    auto res = node.request_vote(args);

    EXPECT_FALSE(res.granted());
  }

  TEST_F(NodeTest, RequestVote_Returns_True_If_Vote_Granted) {
    in_memory_log log;
    log.append(create_log_entry(2));
    log.append(create_log_entry(2));
    log.set_current_term(2);
    InMemoryNode node(log);
    auto args = vote_args(2).candidate(1).last_index_term(2, 2).get();

    auto res = node.request_vote(args);

    EXPECT_TRUE(res.granted());
    EXPECT_EQ(1, log.voted_for());
  }

  TEST_F(NodeTest, RequestVote_Returns_True_If_Already_VotedFor_Same_Candiate) {
    in_memory_log log;
    log.append(create_log_entry(2));
    log.set_current_term(2);
    InMemoryNode node(log);
    auto args = vote_args(2).candidate(1).last_index_term(1, 2).get();
    node.request_vote(args);
    args = vote_args(2).candidate(1).last_index_term(1, 2).get();

    auto res = node.request_vote(args);

    EXPECT_TRUE(res.granted());
  }

  TEST_F(NodeTest, StartElection_IncrementCurrentTerm){
    in_memory_log log;
    log.append(create_log_entry(2));
    log.set_current_term(2);
    InMemoryNode node(log);

    node.start_election();

    EXPECT_EQ(3, log.current_term());
  }

  TEST_F(NodeTest, StartElection_ConvertsToCandidate){
    in_memory_log log;
    InMemoryNode node(log);

    node.start_election();

    EXPECT_EQ(node_state::CANDIDATE, node.state());
  }

  TEST_F(NodeTest, AppendEntries_FromNewLeader_ConvertToFollower) {
    in_memory_log log;
    log.append(create_log_entry(2));
    InMemoryNode node(log);
    node.start_election();
    auto args = append_args(4).prev_index_term(1, 2).get();

    auto res = node.append_entries(args);

    EXPECT_TRUE(res.success());
    EXPECT_EQ(node_state::FOLLOWER, node.state());
  }

  TEST_F(NodeTest, KeepAlive_FromNewLeader_UpdateCurrentTerm) {
    in_memory_log log;
    log.append(create_log_entry(2));
    log.set_current_term(2);
    InMemoryNode node(log);
    auto args = append_args(3).prev_index_term(1, 2).get();

    auto res = node.append_entries(args);

    EXPECT_TRUE(res.success());
    EXPECT_EQ(3, log.current_term());
  }

  TEST_F(NodeTest, RequestVote_FromNewLeader_UpdateCurrentTerm) {
    in_memory_log log;
    log.append(create_log_entry(2));
    log.set_current_term(2);
    InMemoryNode node(log);
    auto args = vote_args(3).candidate(0).last_index_term(1, 0).get();

    node.request_vote(args);

    EXPECT_EQ(3, log.current_term());
  }

  TEST_F(NodeTest, RequestVote_FromNewLeader_ConvertToFollower) {
    in_memory_log log;
    log.append(create_log_entry(2));
    InMemoryNode node(log);
    node.start_election();
    auto args = vote_args(4).candidate(0).last_index_term(1, 0).get();

    node.request_vote(args);

    EXPECT_EQ(node_state::FOLLOWER, node.state());
  }
}
