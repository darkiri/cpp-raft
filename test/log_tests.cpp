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
    log.Append(CreateLogEntry(1));
    log.Append(CreateLogEntry(2));
    log.Append(CreateLogEntry(3));
    log.Trim(log.Begin() + 1);
    EXPECT_EQ(1, (--log.End())->term);
  }
}
