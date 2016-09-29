#ifndef SETTI_PARSER_H
#define SETTI_PARSER_H

#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <boost/format.hpp>

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

  ParserResult<StatementList> AstGen() {
    return ParserStmtList();
  }

  inline uint nerrors() const {
    return nerror_;
  }

  inline const Messages& Msgs() const {
    return msgs_;
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

  // Advance until find a valid token
  inline const Token& ValidToken() {
    while (CurrentToken() == TokenKind::NWL) {
      Advance();
    }

    return token_;
  }

  inline bool CheckComma() {
    if (token_.Is(TokenKind::COMMA)) {
      Advance();
      ValidToken();
      return true;
    } else {
      return false;
    }
  }

  inline std::string TokenValueStr() {
    return Token::TokenValueToStr(token_.GetValue());
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

  ParserResult<Expression> LiteralExp();
  ParserResult<Expression> ParserPrimaryExp();
  ParserResult<Expression> ParserPostExp();
  ParserResult<Expression> ParserUnaryExp();
  ParserResult<Expression> ParserTerm();
  ParserResult<Expression> ParserArithExp();
  ParserResult<Expression> ParserShiftExp();
  ParserResult<Expression> ParserBitAndExp();
  ParserResult<Expression> ParserBitXorExp();
  ParserResult<Expression> ParserBitOrExp();
  ParserResult<Expression> ParserComparisonExp();
  ParserResult<Expression> ParserNotExp();
  ParserResult<Expression> ParserAndExp();
  ParserResult<Expression> ParserOrExp();
  ParserResult<ExpressionList> ParserExpList();
  ParserResult<Statement> ParserStmt();
  ParserResult<Statement> ParserSimpleStmt();
  ParserResult<StatementList> ParserStmtList();
  ParserResult<Statement> ParserBlock();
  ParserResult<Statement> ParserBreakStmt();
  ParserResult<Statement> ParserIfStmt();
  ParserResult<Statement> ParserWhileStmt();

  TokenStream ts_;
  AstNodeFactory factory_;
  uint nerror_;
  Token& token_;
  Messages msgs_;
};

}
}

#endif  // SETTI_PARSER_H

