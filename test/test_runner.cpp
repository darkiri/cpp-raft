#include <gtest/gtest.h>
#include "logging.h"

int main(int argc, char **argv) {
  init_log();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
