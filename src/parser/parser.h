#ifndef SETTI_PARSER_H
#define SETTI_PARSER_H

#include <string>
#include <memory>
#include <vector>
#include <iostream>

#include "token.h"
#include "msg.h"
#include "lexer.h"

namespace setti {
namespace internal {

class Ast {};

template<class T>
class ParserResult {

};

class Parser {
 public:
  Parser(TokenStream&& ts): ts_(std::move(ts)) {}

 private:
  inline const Token& CurrentToken() const noexcept {
    return ts_.CurrentToken();
  }

  inline const Token& PeekAhead() const noexcept {
    return ts_.PeekAhead();
  }

  inline void Advance() noexcept {
    ts_.Advance();
  }

  inline const Token& NextToken() noexcept {
    return ts_.NextToken();
  }

  ParserResult<Ast> ParserPrimaryExpr();
  ParserResult<Ast> ParserPostExpr();

  TokenStream ts_;
};

}
}

#endif  // SETTI_PARSER_H

