#include <gtest/gtest.h>
#include <node.h>
#include <vector>
#include <memory>

using namespace std;

namespace raft {

  class InMemoryLog : public Log {
    public:
      InMemoryLog() {};
      void Append(const LogEntry& entry) {
        auto e = unique_ptr<LogEntry>(new LogEntry(entry));
        _entries.push_back(move(e));
      }
      const LogEntry* GetLastEntry() const {
        return _entries.empty() 
          ? nullptr 
          : _entries.back().get();
      };
      const LogEntry* Get(unsigned int index) const {
        return _entries.size() <= index 
          ? nullptr 
          : _entries[index].get();
      }
      virtual ~InMemoryLog() { };
    private:
      vector<unique_ptr<LogEntry>> _entries;
  };

  class NodeTest : public ::testing::Test {
    protected:
      NodeTest() {
      }

      virtual ~NodeTest() {
      }
  };

  const LogEntry CreateLogEntry(int term) {
    return LogEntry { term };
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
