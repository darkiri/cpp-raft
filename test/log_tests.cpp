#include <gtest/gtest.h>
#include "test_funcs.h"

using namespace std;

namespace raft {
  class InMemoryLogTest : public ::testing::Test {
    protected:
      InMemoryLogTest() {
      }

      virtual ~InMemoryLogTest() {
      }
  };

  TEST_F(InMemoryLogTest, Trim_Test) {
    InMemoryLog log;
    log.Append(create_log_entry(1));
    log.Append(create_log_entry(2));
    log.Append(create_log_entry(3));
    log.Trim(log.Begin() + 1);
    EXPECT_EQ(1, (--log.End())->term());
  }
}
