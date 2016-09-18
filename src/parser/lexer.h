#ifndef SETTI_LEXER_H
#define SETTI_LEXER_H

#include <string>
#include <memory>
#include <vector>

#include "token.h"
#include "msg.h"

namespace setti {
namespace internal {

class Lexer {
 public:
  static const char kEndOfInput = -1;

  Lexer(const std::string& str)
      : str_(str)
      , strlen_(str.length())
      , c_(str_[0])
      , buffer_cursor_(0)
      , line_(0)
      , line_pos_(0)
      , nerror_(0) {}

 private:
  void Scanner();
  void Advance();
  char PeekAhead();
  void Select(TokenKind k);
  void Select(TokenKind k, Token::Value v);
  void SkipSingleLineComment();
  void ScanString();
  char ScanStringEscape();
  void ErrorMsg(const std::string str_msg);

  const std::string str_;
  uint strlen_;
  char c_;
  uint buffer_cursor_;
  uint line_;
  uint line_pos_;
  uint nerror_;
  TokenStream ts_;
  Messages msgs_;

};

}
}

#endif  // SETTI_LEXER_H
