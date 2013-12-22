#include <gtest/gtest.h>
#include <node.h>

using namespace std;

namespace raft {

    class NodeTest : public ::testing::Test {
        protected:
            NodeTest() {
            }

            virtual ~NodeTest() {
            }
    };

    TEST_F(NodeTest, New_Node_Is_Follower) {
        Node node;
        EXPECT_EQ(Follower, node.GetState());
    }
}
