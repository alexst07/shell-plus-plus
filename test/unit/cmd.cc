#include <iostream>
#include <gtest/gtest.h>

#include "cmd-exec.h"

TEST(Token, Print) {
  using namespace shpp::internal;

  SymbolTableStack stack;
  Job job(stack);
  std::vector<std::string> args1 = {"ls"};
  std::vector<std::string> args2 = {"grep", "t"};
  Process p1(stack, std::move(args1)), p2(stack, std::move(args2));

  job.process_.push_back(p1);
  job.process_.push_back(p2);
  job.shell_is_interactive_ = 0;
  job.stderr_ = STDERR_FILENO;
  job.stdout_ = STDOUT_FILENO;
  job.stdin_ = STDIN_FILENO;

  job.LaunchJob(1);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
