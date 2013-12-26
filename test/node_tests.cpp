#include <gtest/gtest.h>
#include <node.h>

using namespace std;

namespace raft {

    class LogStub : public Log {
        public:
            LogStub(int lastTerm): _stubEntry{ lastTerm } {};
            const LogEntry& GetLastEntry() const {
                return _stubEntry;
            };
            virtual ~LogStub() {};
        private:
            const LogEntry _stubEntry;
    };


    class NodeTest : public ::testing::Test {
        protected:
            NodeTest() {
            }

            virtual ~NodeTest() {
            }
    };

    TEST_F(NodeTest, New_Node_Is_Follower) {
        LogStub log(0);
        Node node(log);
        EXPECT_EQ(FOLLOWER, node.GetState());
    }

    TEST_F(NodeTest, AppendEntry_Returns_CurrentTerm) {
        LogStub log(2);
        Node node(log);
        Entry entry{ 1 };
        auto res = node.AppendEntry(entry);
        EXPECT_EQ(2, res.term);
    }

    TEST_F(NodeTest, AppendEntry_Returns_False_If_Term_Is_Lower_As_CurrentTerm) {
        LogStub log(2);
        Node node(log);
        Entry entry{ 1 };
        auto res = node.AppendEntry(entry);
        auto success = res.success;
        EXPECT_FALSE(success);
    }
}
