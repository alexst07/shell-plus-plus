#include <iostream>
#include <gtest/gtest.h>

#include "parser/parser.h"

TEST(Lexer, Check) {
  using namespace setti::internal;

  Lexer l("5 +4* 4+51-85 /78");
  std::cout << "Lexer\n";
  TokenStream ts = l.Scanner();
  Parser p(std::move(ts));
  auto res = p.AstGen();

  if (p.nerrors() == 0) {
    std::cout << "Correct analysis\n";
  } else {
    std::cout << "Parser error analysis\n";
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


