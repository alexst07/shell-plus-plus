#include <iostream>
#include <gtest/gtest.h>

#include "parser/lexer.h"

TEST(Lexer, Check) {
  using namespace shpp::internal;

  Lexer l(":! <= ->rwe\n asdf; \n4.5.7 af/ds /home/alex/tst ./test/qua\\ sdf/sdf.txt\n"
          "if test == 4.48 { while (true) {echo \"aasdf\"}}");
  std::cout << "Lexer\n";
  TokenStream ts = l.Scanner();
  std::cout << "Scanner\n";
  do {
    std::cout << "loop\n";
    Token t = ts.CurrentToken();
    std::cout << t;
  } while (ts.Advance());
}

TEST(Lexer, Comment) {
  using namespace shpp::internal;

  Lexer l(" for asdf in sdf[4]\\\n hello {# comment\n echo oi}");
  std::cout << "Lexer\n";
  TokenStream ts = l.Scanner();
  std::cout << "Scanner\n";
  do {
    std::cout << "loop\n";
    Token t = ts.CurrentToken();
    std::cout << t;
  } while (ts.Advance());
}

TEST(Lexer, Error) {
  using namespace shpp::internal;

  Lexer l("\"asdfsd\n in sdf[4] {# comment\n echo oi}");
  std::cout << "Lexer\n";
  TokenStream ts = l.Scanner();
  std::cout << "Scanner\n";
  do {
    std::cout << "loop\n";
    Token t = ts.CurrentToken();
    std::cout << t;
  } while (ts.Advance());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

