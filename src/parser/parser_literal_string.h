#ifndef SHPP_PARSER_LITERAL_STRING_H
#define SHPP_PARSER_LITERAL_STRING_H

#include <exception>
#include <string>
#include <vector>

#include "parser/lexer.h"

namespace shpp {
namespace internal {

class ParserLiteralStringError : public std::exception {
 public:
  ParserLiteralStringError();

  /**
   * @return the error description and the context as a text string.
   */
  virtual const char* what() const noexcept { return msg_.c_str(); }

  const std::string& msg() const noexcept { return msg_; }

  std::string msg_;
};

class LiteralStringToken {
 public:
  LiteralStringToken(const std::string str_token, bool interpretable)
      : str_token_(str_token), interpretable_(interpretable) {}

  LiteralStringToken(LiteralStringToken&& lit_str_tk)
      : str_token_(std::move(lit_str_tk.str_token_)),
        interpretable_(lit_str_tk.interpretable_) {}

  const std::string& GetStrToken() const noexcept { return str_token_; }

  std::string& GetStrToken() noexcept { return str_token_; }

  bool IsInterpretable() const noexcept { return interpretable_; }

 private:
  std::string str_token_;
  bool interpretable_;
};

class ParserLiteralString {
 public:
  ParserLiteralString(const std::string& literal_str)
      : literal_str_(literal_str),
        interpreter_mode_(false),
        braces_count_(0),
        string_inside_mode_(false),
        escape_mode_(false),
        cmd_mode_(false),
        pos_(0) {}
  void Scanner();

  std::vector<LiteralStringToken>& getStringTokens() { return str_tokens_; }

  const std::vector<LiteralStringToken>& getStringTokens() const noexcept {
    return str_tokens_;
  }

 private:
  char getChar() const noexcept { return literal_str_[pos_]; }

  bool isEnd() const noexcept { return pos_ >= literal_str_.length(); }

  void nextAndPushChar() noexcept {
    if (!isEnd()) str_token_ += literal_str_[pos_++];
  }

  void pushStrToken(bool isInterpretable) {
    LiteralStringToken tk(str_token_, isInterpretable);
    str_tokens_.push_back(std::move(tk));
    str_token_ = "";
  }

  void parserNotInterpretableStr();

  std::string literal_str_;
  bool interpreter_mode_;
  std::string str_token_;
  size_t braces_count_;
  bool string_inside_mode_;
  bool escape_mode_;
  bool cmd_mode_;
  size_t pos_;

  std::vector<LiteralStringToken> str_tokens_;
};

}  // namespace internal
}  // namespace shpp
#endif