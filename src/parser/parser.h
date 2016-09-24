#ifndef SETTI_PARSER_H
#define SETTI_PARSER_H

#include <string>
#include <memory>
#include <vector>
#include <iostream>

#include "token.h"
#include "msg.h"
#include "lexer.h"
#include "parser_result.h"
#include "ast/ast.h"

namespace setti {
namespace internal {

class Parser {
 public:
  Parser() = delete;

  Parser(TokenStream&& ts)
      : ts_(std::move(ts))
      , factory_(std::bind(&Parser::Pos, this))
      , nerror_(0)
      , token_(ts_.CurrentToken()) {}

  ParserResult<Expression> AstGen() {
    return ParserSimpleExp();
  }

  inline uint nerrors() const {
    return nerror_;
  }

 private:
  inline const Token& CurrentToken() const noexcept {
    return ts_.CurrentToken();
  }

  inline const Token& PeekAhead() const noexcept {
    return ts_.PeekAhead();
  }

  inline void Advance() noexcept {
    ts_.Advance();
    token_ = ts_.CurrentToken();
  }

  Token NextToken() noexcept {
    Token token = ts_.NextToken();
    token_ = ts_.CurrentToken();
    return token;
  }

  void ErrorMsg(const boost::format& fmt_msg) {
    Message msg(Message::Severity::ERR, fmt_msg, token_.Line(), token_.Col());
    msgs_.Push(std::move(msg));
    nerror_++;
  }

  inline Position Pos() {
    Position pos = {token_.Line(), token_.Col()};
    return pos;
  }

  ParserResult<Expression> ParserPrimaryExpr();
  ParserResult<Expression> ParserTerm();
  ParserResult<Expression> ParserSimpleExp();

  TokenStream ts_;
  AstNodeFactory factory_;
  uint nerror_;
  Token& token_;
  Messages msgs_;
};

}
}

#endif  // SETTI_PARSER_H

