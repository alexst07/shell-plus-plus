#include <iostream>
#include <gtest/gtest.h>

#include "parser/token.h"

TEST(Token, Print) {
  using namespace setti::internal;

  Token t1(TokenKind::KW_IF, true, 1, 1);
  Token t2(TokenKind::INT_LITERAL, 5, true, 1, 1);
  Token t3(TokenKind::REAL_LITERAL, float(2.54), true, 1, 1);
  Token t4(TokenKind::KW_WHILE, true, 1, 1);

  std::cout << t1 << t2 << t3 << t4;
}

TEST(Token, Comparing) {
  using namespace setti::internal;

  Token t1(TokenKind::KW_IF, "if", true, 1, 1);
  Token t2(TokenKind::INT_LITERAL, 5, true, 1, 1);
  Token t3(TokenKind::REAL_LITERAL, float(2.54), true, 1, 1);

  ASSERT_TRUE(t1 == TokenKind::KW_IF);
  ASSERT_TRUE(t2.Is(TokenKind::INT_LITERAL));
  ASSERT_TRUE(t3 == TokenKind::REAL_LITERAL);

  ASSERT_TRUE(t1 != TokenKind::INT_LITERAL);
  ASSERT_TRUE(t2.IsNot(TokenKind::REAL_LITERAL));
  ASSERT_TRUE(t3 != TokenKind::KW_IF);

  ASSERT_TRUE(t1.IsAny(TokenKind::INT_LITERAL, TokenKind::KW_IF));
  ASSERT_TRUE(t2.IsNot(TokenKind::REAL_LITERAL, TokenKind::KW_IF));
}

TEST(TokenStream, Operations) {
  using namespace setti::internal;

  Token t1(TokenKind::KW_IF, true, 1, 1);
  Token t2(TokenKind::INT_LITERAL, 5, true, 1, 1);
  Token t3(TokenKind::REAL_LITERAL, float(2.54), true, 1, 1);

  TokenStream ts;
  ts.PushToken(std::move(t1));
  ts.PushToken(std::move(t2));
  ts.PushToken(std::move(t3));

  do {
    std::cout << ts.CurrentToken();
  } while (ts.Advance());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
