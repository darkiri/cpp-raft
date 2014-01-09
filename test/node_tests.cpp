#include <gtest/gtest.h>
#include "test_funcs.h"

using namespace std;

namespace raft {

  class NodeTest : public ::testing::Test {
    protected:
      NodeTest() {
      }

      virtual ~NodeTest() {
      }
  };

  Node::AppendEntriesArgs MakeArgs(
      unsigned int term,
      unsigned int prev_log_index,
      unsigned int prev_log_term,
      vector<LogEntry> entries = vector<LogEntry>()){
    return Node::AppendEntriesArgs {term, prev_log_index, prev_log_term, entries, 0};
  }

  TEST_F(NodeTest, New_Node_Is_Follower) {
    InMemoryLog log;
    Node node(log);
    EXPECT_EQ(FOLLOWER, node.GetState());
  }

  TEST_F(NodeTest, AppendEntries_Returns_CurrentTerm) {
    InMemoryLog log;
    log.Append(CreateLogEntry(2));
    Node node(log);
    auto args = MakeArgs(1, 0, 0);
    auto res = node.AppendEntries(args);
    EXPECT_EQ(2, res.term);
  }

  TEST_F(NodeTest, AppendEntries_Returns_False_For_Empty_Log) {
    InMemoryLog log;
    Node node(log);
    auto args = MakeArgs(1, 0, 0);
    auto res = node.AppendEntries(args);
    EXPECT_FALSE(res.success);
  }

  TEST_F(NodeTest, AppendEntries_Returns_False_If_Term_Is_Lower_As_CurrentTerm) {
    InMemoryLog log;
    log.Append(CreateLogEntry(2));
    Node node(log);
    auto args = MakeArgs(1, 0, 0);
    auto res = node.AppendEntries(args);
    EXPECT_FALSE(res.success);
  }

  TEST_F(NodeTest, AppendEntries_Returns_False_If_Log_DoesNotContain_prevLogTerm_At_prevLogIndex) {
    InMemoryLog log;
    log.Append(CreateLogEntry(2));
    log.Append(CreateLogEntry(3));
    Node node(log);
    auto args = MakeArgs(3, 0, 1);
    auto res = node.AppendEntries(args);
    EXPECT_FALSE(res.success);
  }

  TEST_F(NodeTest, AppendEntries_TrimsLog_If_TermDoesNotMatch) {
    InMemoryLog log;
    log.Append(CreateLogEntry(2));
    log.Append(CreateLogEntry(3));
    Node node(log);
    auto entries = vector<LogEntry> {CreateLogEntry(4), CreateLogEntry(5)};
    auto args = MakeArgs(5, 0, 2, entries);
    auto res = node.AppendEntries(args);
    EXPECT_TRUE(res.success);
    EXPECT_EQ(2, log.Get(0)->term);
    EXPECT_EQ(4, log.Get(1)->term);
    EXPECT_EQ(5, log.Get(2)->term);
  }

  TEST_F(NodeTest, AppendEntries_AppendNewEntries) { // yep, this is the name
    InMemoryLog log;
    log.Append(CreateLogEntry(2));
    Node node(log);
    auto entries = vector<LogEntry> {CreateLogEntry(4)};
    auto args = MakeArgs(5, 0, 2, entries);
    auto res = node.AppendEntries(args);
    EXPECT_TRUE(res.success);
    EXPECT_EQ(2, log.Get(0)->term);
    EXPECT_EQ(4, log.Get(1)->term);
  }
}
