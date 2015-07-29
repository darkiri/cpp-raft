#include "node_tests.h"

namespace raft {
  TEST_F(NodeTest, RequestVote_Returns_False_If_Term_Is_Lower_As_CurrentTerm) {
    init_log(*plog_, 2).entry(1).entry(2);
    auto args = vote_args(1).candidate(1).last_index_term(2, 1).get();

    auto res = pnode_->request_vote(args);

    EXPECT_FALSE(res.granted());
  }

  TEST_F(NodeTest, RequestVote_Returns_CurrentTerm) {
    init_log(*plog_, 3).entry(1).entry(2);
    auto args = vote_args(1).candidate(1).last_index_term(1, 1).get();

    auto res = pnode_->request_vote(args);

    EXPECT_EQ(3, res.term());
  }

  TEST_F(NodeTest, RequestVote_Returns_False_If_Already_VotedFor_Another_Candiate) {
    init_log(*plog_, 2).entry(2);
    auto args = vote_args(2).candidate(1).last_index_term(1, 2).get();

    auto res = pnode_->request_vote(args);

    EXPECT_TRUE(res.granted());

    args = vote_args(3).candidate(2).last_index_term(1, 3).get();
    res = pnode_->request_vote(args);

    EXPECT_FALSE(res.granted());
  }

  TEST_F(NodeTest, RequestVote_CandidatesLogWithLaterTerm_Is_UpToDate) {
    init_log(*plog_, 3).entry(1).entry(2).entry(2);
    auto args = vote_args(3).candidate(1).last_index_term(1, 3).get();

    auto res = pnode_->request_vote(args);

    EXPECT_TRUE(res.granted());
  }

  TEST_F(NodeTest, RequestVote_CandidatesLogWithEarlierTerm_Is_Not_UpToDate) {
    init_log(*plog_, 2).entry(2);
    auto args = vote_args(2).candidate(1).last_index_term(2, 1).get();

    auto res = pnode_->request_vote(args);

    EXPECT_FALSE(res.granted());
  }

  TEST_F(NodeTest, RequestVote_CandidatesLogSameTermLonger_Is_UpToDate) {
    init_log(*plog_, 2).entry(1);
    auto args = vote_args(2).candidate(1).last_index_term(2, 1).get();

    auto res = pnode_->request_vote(args);

    EXPECT_TRUE(res.granted());
  }

  TEST_F(NodeTest, RequestVote_CandidatesLogSameTermShorter_Is_Not_UpToDate) {
    init_log(*plog_, 2).entry(1).entry(1);
    auto args = vote_args(2).candidate(1).last_index_term(1, 1).get();

    auto res = pnode_->request_vote(args);

    EXPECT_FALSE(res.granted());
  }

  TEST_F(NodeTest, RequestVote_Returns_True_If_Vote_Granted) {
    init_log(*plog_, 2).entry(2).entry(2);
    auto args = vote_args(2).candidate(1).last_index_term(2, 2).get();

    auto res = pnode_->request_vote(args);

    EXPECT_TRUE(res.granted());
    EXPECT_EQ(1, plog_->voted_for());
  }

  TEST_F(NodeTest, RequestVote_Returns_True_If_Already_VotedFor_Same_Candiate) {
    init_log(*plog_, 2).entry(2);
    auto args = vote_args(2).candidate(1).last_index_term(1, 2).get();

    auto res = pnode_->request_vote(args);

    EXPECT_TRUE(res.granted());

    args = vote_args(2).candidate(1).last_index_term(1, 2).get();
    res = pnode_->request_vote(args);

    EXPECT_TRUE(res.granted());
  }

  TEST_F(NodeTest, RequestVote_FromNewLeader_UpdateCurrentTerm) {
    init_log(*plog_, 2).entry(2);
    auto args = vote_args(3).candidate(0).last_index_term(1, 0).get();

    pnode_->request_vote(args);

    EXPECT_EQ(3, plog_->current_term());
  }

  TEST_F(NodeTest, RequestVote_FromNewLeader_ConvertToFollower) {
    init_log(*plog_, 2).entry(2);

    pnode_->start_election();
    auto args = vote_args(4).candidate(0).last_index_term(1, 0).get();

    pnode_->request_vote(args);

    EXPECT_EQ(node_state::FOLLOWER, pnode_->state());
  }
}
