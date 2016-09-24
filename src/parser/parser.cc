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

  if (token_.IsAny(TokenKind::ADD, TokenKind::SUB)) {
    TokenKind token_kind = token_.GetKind();
    Advance();
    rexp = std::move(ParserSimpleExp());

    if (rexp) {
      return ParserResult<Expression>(factory_.NewBinaryOperation(
          token_kind, std::move(lexp.MoveAstNode()),
          std::move(rexp.MoveAstNode())));
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
    TokenKind token_kind = token_.GetKind();
    Advance();
    rexp = std::move(ParserTerm());

    if (rexp) {
      return ParserResult<Expression>(factory_.NewBinaryOperation(
          token_kind, std::move(lexp.MoveAstNode()),
          std::move(rexp.MoveAstNode())));
    } else {
      return ParserResult<Expression>(); // Error
    }
  }

  return lexp;
}

ParserResult<Expression> Parser::ParserPrimaryExpr() {
  Token token(CurrentToken());
  Advance();
  if (token.Is(TokenKind::INT_LITERAL)) {
    return ParserResult<Expression>(factory_.NewLiteral(token.GetValue(),
                                                        Literal::kInteger));
  } else if (token.Is(TokenKind::STRING_LITERAL)) {
    return ParserResult<Expression>(factory_.NewLiteral(token.GetValue(),
                                                        Literal::kString));
  } else if (token.Is(TokenKind::REAL_LITERAL)) {
    return ParserResult<Expression>(factory_.NewLiteral(token.GetValue(),
                                                        Literal::kReal));
  } else if (token.IsAny(TokenKind::KW_TRUE, TokenKind::KW_FALSE)) {
    return ParserResult<Expression>(factory_.NewLiteral(token.GetValue(),
                                                        Literal::kBool));
  } else {
    ErrorMsg(boost::format("primary expression expected"));
    return ParserResult<Expression>(); // Error
  }
}

}
}
