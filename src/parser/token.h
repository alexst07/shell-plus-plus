#ifndef SETTI_TOKEN_H
#define SETTI_TOKEN_H

#include <string>
#include <memory>
#include <boost/variant.hpp>

namespace setti {

enum class TokenKind {
  unknown = 0,
  eof,
  comment,
  int_literal,
  hex_literal,
  oct_literal,
  bin_literal,
  real_literal,
  string_literal,

#define KEYWORD(X) kw_ ## X,
#define PUNCTUATOR(X, Y) X,
#define POUND_KEYWORD(X) pound_ ## X,
#include "token.def"

  NUM_TOKENS
};

class Token {
 public:
  using Value = boost::variant<int, std::string, float, bool>;

  Token(TokenKind k, Value value, bool blank_after, uint line, uint col)
      : kind_(k)
      , value_(value)
      , blank_after_(blank_after)
      , line_(line)
      , col_(col) {}

  Token(const Token& tok)
      : kind_(tok.kind_)
      , value_(tok.value_)
      , blank_after_(tok.blank_after_)
      , line_(tok.line_)
      , col_(tok.col_) {}

  Token(Token&& tok) noexcept
      : kind_(tok.kind_)
      , value_(std::move(tok.value_))
      , blank_after_(tok.blank_after_)
      , line_(tok.line_)
      , col_(tok.col_) {}

  Token& operator=(const Token& tok) {
    if (&tok == this)
      return *this;

    kind_ = tok.kind_;
    value_ = tok.value_;
    blank_after_ = tok.blank_after_;
    line_ = tok.line_;
    col_ = tok.col_;
  }

  Token& operator=(Token&& tok) noexcept {
    if (&tok == this)
      return *this;

    kind_ = tok.kind_;
    value_ = std::move(tok.value_);
    blank_after_ = tok.blank_after_;
    line_ = tok.line_;
    col_ = tok.col_;
  }

  bool operator==(TokenKind k) const noexcept { return Is(k); }
  bool operator!=(TokenKind k) const noexcept { return IsNot(k); }

  TokenKind GetKind() const noexcept { return kind_; }

  Value GetValue() const noexcept { return value_; }

  bool BlankAfter() const noexcept { return blank_after_; }

  uint Line() const noexcept { return line_; }

  uint Col() const noexcept { return col_; }

  bool Is(TokenKind k) const noexcept { return kind_ == k; }

  bool IsNot(TokenKind k) const noexcept { return kind_ != k; }

  bool IsAny(TokenKind k) const noexcept {
    return Is(k);
  }

  template <typename ...T>
  bool IsAny(TokenKind k1, TokenKind k2, T... kn) const noexcept {
    if (Is(k1))
      return true;
    return IsAny(k2, kn...);
  }

  template <typename ...T>
  bool isNot(TokenKind k1, T... kn) const noexcept {
    return !IsAny(k1, kn...);
  }

 private:
  TokenKind kind_;
  bool blank_after_;
  Value value_;
  uint line_;
  uint col_;
};

}

#endif  // SETTI_TOKEN_H
