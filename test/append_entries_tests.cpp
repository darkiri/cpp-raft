#include "node_tests.h"

namespace raft {
  TEST_F(NodeTest, AppendEntries_Returns_CurrentTerm) {
    init_log(*plog_, 3).entry(1).entry(3);
    auto args = append_args(2).prev_index_term(1, 2).get();

    auto res = pnode_->append_entries(args);

    EXPECT_EQ(3, res.term());
  }

  TEST_F(NodeTest, AppendEntries_Returns_UpdatedCurrentTerm) {
    init_log(*plog_, 2).entry(1).entry(2);
    auto args = append_args(3).prev_index_term(1, 1).get();

    auto res = pnode_->append_entries(args);

    EXPECT_EQ(3, res.term());
  }

  TEST_F(NodeTest, AppendEntries_Returns_True_For_Empty_Log) {
    auto args = append_args(1).get();

    auto res = pnode_->append_entries(args);

    EXPECT_TRUE(res.success());
  }

  TEST_F(NodeTest, AppendEntries_Returns_False_If_Term_Is_Lower_As_CurrentTerm) {
    init_log(*plog_, 3).entry(1).entry(2);
    auto args = append_args(2).prev_index_term(1, 1).get();

    auto res = pnode_->append_entries(args);

    EXPECT_FALSE(res.success());
  }

  TEST_F(NodeTest, AppendEntries_Returns_True_If_Term_Is_Same_As_CurrentTerm) {
    init_log(*plog_, 2).entry(1);
    auto args = append_args(2).prev_index_term(1, 1).get();

    auto res = pnode_->append_entries(args);

    EXPECT_TRUE(res.success());
  }

  TEST_F(NodeTest, AppendEntries_Returns_False_If_Log_Not_Matching) {
    init_log(*plog_, 3).entry(1).entry(2).entry(3).entry(3);
    auto args = append_args(3).prev_index_term(2, 1).get();

    auto res = pnode_->append_entries(args);

    EXPECT_FALSE(res.success());
  }

  TEST_F(NodeTest, AppendEntries_Returns_False_If_Log_Not_Matching_WithMissingEntries) {
    init_log(*plog_, 3).entry(1).entry(2).entry(3);
    auto args = append_args(5).prev_index_term(10, 4).get();

    auto res = pnode_->append_entries(args);

    EXPECT_FALSE(res.success());
  }

  TEST_F(NodeTest, AppendEntries_AppendsNewEntries) { // yep, this is the name
    init_log(*plog_, 2).entry(2);
    auto args = append_args(5).prev_index_term(1, 2).log_entry(4).get();

    auto res = pnode_->append_entries(args);

    EXPECT_TRUE(res.success());
    EXPECT_EQ(3, plog_->size());
    ExpectLogTerm(1, 2);
    ExpectLogTerm(2, 4);
  }

  TEST_F(NodeTest, AppendEntries_TrimsLog_If_TermDoesNotMatch) {
    init_log(*plog_, 3).entry(1).entry(2).entry(2).entry(3);
    auto args = append_args(5).prev_index_term(2, 2).log_entry(4).log_entry(5).get();

    auto res = pnode_->append_entries(args);

    EXPECT_TRUE(res.success());
    EXPECT_EQ(5, plog_->size());
    ExpectLogTerm(1, 1);
    ExpectLogTerm(2, 2);
    ExpectLogTerm(3, 4);
    ExpectLogTerm(4, 5);
  }

  TEST_F(NodeTest, AppendEntries_KeepAlive_Does_Not_AppendEntries) {
    init_log(*plog_, 2).entry(1).entry(2);
    auto args = append_args(2).prev_index_term(2, 2).get();

    auto res = pnode_->append_entries(args);

    EXPECT_TRUE(res.success());
    EXPECT_EQ(3, plog_->size());
    ExpectLogTerm(1, 1);
    ExpectLogTerm(2, 2);
  }


  TEST_F(NodeTest, AppendEntries_KeepAlive_Updates_CurrentTerm) {
    init_log(*plog_, 2).entry(1).entry(2);
    auto args = append_args(3).prev_index_term(2, 2).get();

    auto res = pnode_->append_entries(args);

    EXPECT_EQ(3, plog_->current_term());
  }

  TEST_F(NodeTest, AppendEntries_Updates_CurrentTerm) {
    init_log(*plog_, 2).entry(2);
    auto args = append_args(3).prev_index_term(1, 2).log_entry(3).get();

    pnode_->append_entries(args);

    EXPECT_EQ(3, plog_->current_term());
  }

  TEST_F(NodeTest, AppendEntries_LogsNotMatching_DoNotUpdate_CommitIndex) {
    init_log(*plog_, 1).entry(1);
    auto args = append_args(1).prev_index_term(1, 2)
      .log_entry(1).log_entry(1)
      .commit_index(2).get();

    pnode_->append_entries(args);

    EXPECT_EQ(0, pnode_->commit_index());
  }

  TEST_F(NodeTest, AppendEntries_Set_CommitIndex_To_LeaderCommiIndex) {
    init_log(*plog_, 1).entry(1);
    auto args = append_args(1).prev_index_term(1, 1)
      .log_entry(1).log_entry(1)
      .commit_index(2).get();

    pnode_->append_entries(args);

    EXPECT_EQ(2, pnode_->commit_index());
  }

  TEST_F(NodeTest, AppendEntries_CommitIndexChanged_ApplyToStateMachine) {
    init_log(*plog_, 1).entry(1);
    auto args = append_args(2).prev_index_term(1, 1)
      .log_entry(2).log_entry(2)
      .commit_index(1).get();

    EXPECT_EQ(nullptr, pstate_machine_->last_applied());

    pnode_->append_entries(args);

    EXPECT_EQ(1, pstate_machine_->last_applied()->term());
  }

  TEST_F(NodeTest, AppendEntries_CommitIndexChanged_ApplySeveralEntriesToStateMachine) {
    init_log(*plog_, 1).entry(1);
    auto args = append_args(2).prev_index_term(1, 1)
      .log_entry(2)
      .commit_index(2).get();

    pnode_->append_entries(args);

    EXPECT_EQ(1, pstate_machine_->first_applied()->term());
    EXPECT_EQ(2, pstate_machine_->last_applied()->term());
  }

  TEST_F(NodeTest, AppendEntries_Set_CommitIndex_To_LastNewEntryIndex) {
    init_log(*plog_, 1).entry(1);
    auto args = append_args(1).prev_index_term(1, 1)
      .log_entry(1)
      .commit_index(3).get();

    pnode_->append_entries(args);

    EXPECT_EQ(2, pnode_->commit_index());
  }

  TEST_F(NodeTest, AppendEntries_LeaderCommitLower_CommitIndexNotChanged) {
    init_log(*plog_, 1).entry(1);
    auto args = append_args(1).prev_index_term(1, 1)
      .log_entry(1).log_entry(1)
      .commit_index(3).get();

    pnode_->append_entries(args);

    args = append_args(1).prev_index_term(1, 1)
      .log_entry(1)
      .commit_index(2).get();

    pnode_->append_entries(args);

    EXPECT_EQ(3, pnode_->commit_index());
  }

  TEST_F(NodeTest, AppendEntries_FromNewLeader_ConvertToFollower) {
    init_log(*plog_, 2).entry(2);

    pnode_->start_election();
    auto args = append_args(4).prev_index_term(1, 2).get();

    auto res = pnode_->append_entries(args);

    EXPECT_TRUE(res.success());
    EXPECT_EQ(node_state::FOLLOWER, pnode_->state());
  }
}
