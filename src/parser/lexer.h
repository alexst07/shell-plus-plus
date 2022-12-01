// Copyright 2016 Alex Silva Torres
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SHPP_LEXER_H
#define SHPP_LEXER_H

#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "msg.h"
#include "token.h"

namespace shpp {
namespace internal {

class Lexer {
 public:
  static const char kEndOfInput = -1;

  Lexer(const std::string& str)
      : str_(str),
        strlen_(str.length()),
        c_(str_[0]),
        buffer_cursor_(0),
        line_(1),
        line_pos_(1),
        nerror_(0) {}

  TokenStream Scanner();

  inline uint NumErrors() noexcept { return nerror_; }

  inline const Messages& GetMessages() const noexcept { return msgs_; }

  inline Messages& GetMessages() noexcept { return msgs_; }

 private:
  void SkipSingleLineComment();
  Token ScanString(char string_end_char = '"');
  Token ScanWord(const std::string& prestr = "");
  Token ScanNumber();
  std::string ScanStringEscape();
  char ScanWordEscape();
  char ScanAnsiEscapeCode();
  std::string ScanUnicodeEscapeCode();

  inline bool IsLetter(char c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
  }

  inline bool IsOctalChar(char c) { return (c >= '0' && c < '8'); }

  inline bool IsHexChar(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
  }

  inline bool IsDigit(char c) { return c >= '0' && c <= '9'; }

  inline bool IsIdentifierStart(char c) { return (IsLetter(c) || c == '_'); }

  inline bool IsSpecialChar(char c) {
    bool b = c != ' ' && c != '\t' && c != '\n' && c != ')' && c != ';' &&
             c != '}' && c != '|' && c != '&' && c != '%' && c != kEndOfInput;
    return b;
  }

  inline void Advance() {
    if (buffer_cursor_ == strlen_ - 1) {
      c_ = kEndOfInput;
      return;
    }

    // Check new line and ser cursor position
    if (c_ == '\n') {
      line_++;
      line_pos_ = 0;
    }

    c_ = str_[++buffer_cursor_];

    // Always increment line position, because the first char on line is '1'
    line_pos_++;
  }

  inline void Back() { --buffer_cursor_; }

  inline char PeekAhead() {
    if ((buffer_cursor_ + 1) == strlen_) return kEndOfInput;

    return str_[buffer_cursor_ + 1];
  }

  inline Token GetToken(TokenKind k, char check_blank = 0) {
    if (check_blank == 0) {
      check_blank = c_;
    }

    bool blank_after = check_blank == ' ';

    Token t(k, blank_after, line_, start_pos_);
    return t;
  }

  // Make a new token, but it doesn't advance the cursor
  inline Token GetToken(TokenKind k, Token::Value v, char check_blank = 0) {
    if (check_blank == 0) {
      check_blank = c_;
    }

    bool blank_after = check_blank == ' ';
    Token t(k, v, blank_after, line_, start_pos_);
    return t;
  }

  // Make a new token and advance the cursor
  inline Token Select(TokenKind k, char check_blank = 0) {
    if (check_blank == 0) {
      check_blank = PeekAhead();
    }

    Token t(GetToken(k, check_blank));
    Advance();
    return t;
  }

  inline Token Select(TokenKind k, Token::Value v, char check_blank = 0) {
    if (check_blank == 0) {
      check_blank = PeekAhead();
    }

    Token t(GetToken(k, v, check_blank));
    Advance();
    return t;
  }

  Token ScanIdentifier(bool varenv = false);

  void ErrorMsg(const boost::format& fmt_msg);

  std::string str_;
  uint strlen_;
  char c_;
  uint buffer_cursor_;
  uint line_;
  uint line_pos_;
  uint start_pos_;
  uint nerror_;
  Messages msgs_;
};

}  // namespace internal
}  // namespace shpp

#endif  // SHPP_LEXER_H
