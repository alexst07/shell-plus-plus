#include <iostream>
#include <gtest/gtest.h>

#include "parser/parser.h"
#include "ast/ast-printer.h"

TEST(Lexer, Check) {
  using namespace seti::internal;

  Lexer l("z = (6 << 2), 5 +4* -4+51-(\n\n\n\"oi\" - wef[6*2]->sef[es[\n5+1\n]]) /\n78+t->p(\n4*3,\n ad[4])\n\np[1], a = a && b,t[p]->f()");
  std::cout << "Lexer\n";
  TokenStream ts = l.Scanner();
  Parser p(std::move(ts));
  auto res = p.AstGen();

  if (p.nerrors() == 0) {
    std::cout << "Correct analysis\n";
    AstPrinter visitor;
    visitor.Visit(res.NodePtr());
  } else {
    std::cout << "Parser error analysis\n";
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


