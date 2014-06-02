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
    in_memory_log log;
    log.append(create_log_entry(1));
    log.append(create_log_entry(2));
    log.append(create_log_entry(3));
    log.trim(log.begin() + 1);
    EXPECT_EQ(1, (--log.end())->term());
  }
}
