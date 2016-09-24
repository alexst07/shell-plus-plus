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
  ParserResult<Expression> lexp = ParserPrimaryExp();

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

ParserResult<Expression> Parser::ParserPrimaryExp() {
  Token token(ValidToken());
  if (token == TokenKind::IDENTIFIER) {
    ParserResult<Expression> res = ParserResult<Expression>(
        factory_.NewIdentifier(boost::get<std::string>(token.GetValue())));
    Advance(); // Consume the token
    return res;
  } else if (token == TokenKind::LPAREN) {
    Advance(); // consume the token '('
    ParserResult<Expression> res = ParserSimpleExp();
    if (CurrentToken() != TokenKind::RPAREN) {
      ErrorMsg(boost::format("Expected ')' in the end of expression"));
      return ParserResult<Expression>(); // Error
    }

    Advance(); // consume the token ')'
    return res;
  } else {
    return LiteralExp();
  }

}

ParserResult<Expression> Parser::LiteralExp() {
  Token token(ValidToken());
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
