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

  AppendEntriesArgs MakeArgs(
      unsigned int term,
      unsigned int prev_log_index,
      unsigned int prev_log_term,
      vector<LogEntry> entries = vector<LogEntry>()){
    return AppendEntriesArgs {term, prev_log_index, prev_log_term, entries, 0};
  }

  void ExpectLogSize(Log& log, int s) {
    EXPECT_EQ(s, log.Size());
  }
  void ExpectLogTerm(Log& log, int i, int t) {
    EXPECT_EQ(t, log.Get(i)->term);
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

  TEST_F(NodeTest, AppendEntries_Returns_True_If_Term_Is_Same_As_CurrentTerm) {
    InMemoryLog log;
    log.Append(CreateLogEntry(2));
    Node node(log);
    auto args = MakeArgs(2, 0, 0);
    auto res = node.AppendEntries(args);
    EXPECT_TRUE(res.success);
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
    ExpectLogSize(log, 3);
    ExpectLogTerm(log, 0, 2);
    ExpectLogTerm(log, 1, 4);
    ExpectLogTerm(log, 2, 5);
  }

  TEST_F(NodeTest, AppendEntries_AppendsNewEntries) { // yep, this is the name
    InMemoryLog log;
    log.Append(CreateLogEntry(2));
    Node node(log);
    auto entries = vector<LogEntry> {CreateLogEntry(4)};
    auto args = MakeArgs(5, 0, 2, entries);
    auto res = node.AppendEntries(args);
    EXPECT_TRUE(res.success);
    ExpectLogSize(log, 2);
    ExpectLogTerm(log, 0, 2);
    ExpectLogTerm(log, 1, 4);
  }

  TEST_F(NodeTest, AppendEntries_KeepAlive_Does_Not_AppendEntries) {
    InMemoryLog log;
    log.Append(CreateLogEntry(2));
    Node node(log);
    auto entries = vector<LogEntry>();
    auto args = MakeArgs(2, 0, 2, entries);
    auto res = node.AppendEntries(args);
    EXPECT_TRUE(res.success);
    ExpectLogSize(log, 1);
    ExpectLogTerm(log, 0, 2);
  }
}
