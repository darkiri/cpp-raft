#include <gtest/gtest.h>
#include "test_funcs.h"

namespace raft {

  class NodeTest : public ::testing::Test {
    protected:
      NodeTest() {
      }

      virtual ~NodeTest() {
      }
  };

  TEST_F(NodeTest, New_Node_Is_Follower) {
    InMemoryLog log;
    Node node(log);
    EXPECT_EQ(FOLLOWER, node.GetState());
  }

  TEST_F(NodeTest, AppendEntries_Returns_CurrentTerm) {
    InMemoryLog log;
    log.Append(CreateLogEntry(2));
    Node node(log);
    auto res = node.AppendEntries(1, 0, 0);
    EXPECT_EQ(2, res.term);
  }

  TEST_F(NodeTest, AppendEntries_Returns_False_For_Empty_Log) {
    InMemoryLog log;
    Node node(log);
    auto res = node.AppendEntries(1, 0, 0);
    EXPECT_FALSE(res.success);
  }

  TEST_F(NodeTest, AppendEntries_Returns_False_If_Term_Is_Lower_As_CurrentTerm) {
    InMemoryLog log;
    log.Append(CreateLogEntry(2));
    Node node(log);
    auto res = node.AppendEntries(1, 0, 0);
    EXPECT_FALSE(res.success);
  }

  TEST_F(NodeTest, AppendEntries_Returns_False_If_Log_DoesNotContain_prevLogTerm_At_prevLogIndex) {
    InMemoryLog log;
    log.Append(CreateLogEntry(2));
    log.Append(CreateLogEntry(3));
    Node node(log);
    auto res = node.AppendEntries(3, 0, 1);
    EXPECT_FALSE(res.success);
  }
}
