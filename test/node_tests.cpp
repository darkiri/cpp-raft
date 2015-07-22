#include "node_tests.h"

namespace raft {

  TEST_F(NodeTest, New_Node_Is_Follower) {
    EXPECT_EQ(node_state::FOLLOWER, pnode_->state());
  }

  TEST_F(NodeTest, New_Node_Commit_Index_0) {
    EXPECT_EQ(0, pnode_->commit_index());
  }

  TEST_F(NodeTest, StartElection_IncrementCurrentTerm){
    init_log(*plog_, 2).entry(2);

    pnode_->start_election();

    EXPECT_EQ(3, plog_->current_term());
  }

  TEST_F(NodeTest, StartElection_ConvertsToCandidate){
    pnode_->start_election();

    EXPECT_EQ(node_state::CANDIDATE, pnode_->state());
  }

  TEST_F(NodeTest, KeepAlive_FromNewLeader_UpdateCurrentTerm) {
    init_log(*plog_, 2).entry(2);
    auto args = append_args(3).prev_index_term(1, 2).get();

    auto res = pnode_->append_entries(args);

    EXPECT_TRUE(res.success());
    EXPECT_EQ(3, plog_->current_term());
  }

}
