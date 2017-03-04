#include <iostream>
#include <string>
#include <gtest/gtest.h>

#include "interpreter/cmd-executor.h"

TEST(Token, Print) {
  using namespace shpp::internal;
  std::cout << "adfsdf";
  std::cout << "adfsdf";
  std::cout << "adfsdf";
  std::cout << "adfsdf";
  std::cout << "adfsdf";
  std::cout << "adfsdf";




}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
