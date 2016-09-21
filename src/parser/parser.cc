#include "parser.h"

#include <sstream>

namespace setti {
namespace internal {

ParserResult<Expression> Parser::ParserSimpleExp() {
  ParserResult<Expression> rexp;
  ParserResult<Expression> lexp = ParserTerm();

  if (!lexp) {
    return ParserResult<Expression>(); // Error
  }

  if (CurrentToken().IsAny(TokenKind::ADD, TokenKind::SUB)) {
    Advance();
    rexp = std::move(ParserSimpleExp());

    if (rexp) {
      return ParserResult<Expression>(factory_.NewBinaryOperation(
          CurrentToken().GetKind(), std::move(lexp.MoveAstNode()),
          std::move(rexp.MoveAstNode()), 0));
    }
  }

  return lexp;
}

ParserResult<Expression> Parser::ParserTerm() {
  ParserResult<Expression> rexp;
  ParserResult<Expression> lexp = ParserPrimaryExpr();

  if (!lexp) {
    return ParserResult<Expression>(); // Error
  }

  if (token_.IsAny(TokenKind::MUL, TokenKind::DIV)) {
    Advance();
    rexp = std::move(ParserTerm());

    if (rexp) {
      return ParserResult<Expression>(factory_.NewBinaryOperation(
          CurrentToken().GetKind(), std::move(lexp.MoveAstNode()),
          std::move(rexp.MoveAstNode()), 0));
    } else {
      return ParserResult<Expression>(); // Error
    }
  }

  return lexp;
}

ParserResult<Expression> Parser::ParserPrimaryExpr() {
  Token token(NextToken());
  if (token.Is(TokenKind::INT_LITERAL)) {
    return ParserResult<Expression>(factory_.NewLiteral(token, 0));
  } else if (token.Is(TokenKind::STRING_LITERAL)) {
    return ParserResult<Expression>(factory_.NewLiteral(token, 0));
  } else if (token.Is(TokenKind::REAL_LITERAL)) {
    return ParserResult<Expression>(factory_.NewLiteral(token, 0));
  } else if (token.Is(TokenKind::KW_TRUE)) {
    return ParserResult<Expression>(factory_.NewLiteral(token, 0));
  } else if (token.Is(TokenKind::KW_FALSE)) {
    return ParserResult<Expression>(factory_.NewLiteral(token, 0));
  } else if (token.Is(TokenKind::IDENTIFIER)) {
    return ParserResult<Expression>(factory_.NewLiteral(token, 0));
  } else {
    ErrorMsg(boost::format("primary expression expected"));
    return ParserResult<Expression>(); // Error
  }
}

}
}
