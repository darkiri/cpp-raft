#ifndef node_tests
#define node_tests

#include <gtest/gtest.h>
#include "test_funcs.h"

using namespace std;

namespace raft {
  typedef node<in_memory_log> InMemoryNode;

  class NodeTest : public ::testing::Test {
    protected:
      NodeTest() :
        plog_(new in_memory_log()),
        pnode_(new InMemoryNode(*plog_)) {}

      std::unique_ptr<in_memory_log> plog_;
      std::unique_ptr<InMemoryNode> pnode_;

      void ExpectLogTerm(int index, int expectedTerm) {
        EXPECT_EQ(expectedTerm, plog_->begin()[index].term());
      }
  };
}
#endif
