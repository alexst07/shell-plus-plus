#ifndef SETTI_TOKEN_H
#define SETTI_TOKEN_H

#include <string>
#include <memory>
#include <vector>
#include <boost/variant.hpp>

namespace setti {

enum class TokenKind {
  UNKNOWN = 0,
  EOS,
  COMMENT,
  INT_LITERAL,
  HEX_LITERAL,
  OCT_LITERAL,
  BIN_LITERAL,
  REAL_LITERAL,
  STRING_LITERAL,

#define KEYWORD(X) KW_ ## X,
#define PUNCTUATOR(X, Y) X,
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

  Token(TokenKind k, const char* value, bool blank_after, uint line, uint col)
      : kind_(k)
      , value_(std::string(value))
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

  friend std::ostream& operator<<(std::ostream& stream, Token& token);

 private:
  struct Output : public boost::static_visitor<> {
    std::ostream& stream_;
    Output(std::ostream& stream): stream_(stream) {}
    template <typename T>
    void operator()(T t) const { stream_ << t; }
  };

  TokenKind kind_;
  bool blank_after_;
  Value value_;
  uint line_;
  uint col_;
};

std::ostream& operator<<(std::ostream& stream, Token& token) {
  stream << "Type: " << static_cast<int>(token.kind_) << ", Value: ";
  boost::apply_visitor(Token::Output{stream}, token.value_);
  stream << "\n";

  return stream;
}

class TokenStream {
 public:
  TokenStream(): pos_(0) {}

  TokenStream(const TokenStream&) = delete;
  TokenStream& operator=(const TokenStream&) = delete;

  TokenStream(TokenStream&& tok_stream)
      : tok_vec_(std::move(tok_stream.tok_vec_))
      , pos_(tok_stream.pos_) {
    tok_stream.pos_ = 0;
  }

  TokenStream& operator=(TokenStream&& tok_stream) {
    if (this == &tok_stream)
      return *this;

    tok_stream = std::move(tok_stream);
    pos_ = tok_stream.pos_;
    tok_stream.pos_ = 0;

    return *this;
  }

  void PushToken(Token&& tok) {
    tok_vec_.push_back(std::move(tok));
  }

  const Token& CurrentToken() const {
    return tok_vec_.at(pos_);
  }

  Token& CurrentToken() {
    return tok_vec_.at(pos_);
  }

  const Token& PeekAhead() const {
    return tok_vec_.at(pos_ + 1);
  }

  Token& PeekAhead() {
    return tok_vec_.at(pos_ + 1);
  }

  Token& NextToken() {
    return tok_vec_.at(pos_++);
  }

  bool Advance() {
    if (pos_ == tok_vec_.size() - 1)
      return false;

    ++pos_;
    return true;
  }

  size_t Size() const noexcept {
    return tok_vec_.size();
  }

 private:
  size_t pos_;
  std::vector<Token> tok_vec_;
};

}

#endif  // SETTI_TOKEN_H
