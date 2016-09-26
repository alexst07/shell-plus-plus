#include "parser.h"

#include <sstream>

namespace setti {
namespace internal {

ParserResult<Statement> Parser::ParserAssignStmt() {
  if (token_ != TokenKind::IDENTIFIER) {
    ErrorMsg(boost::format("identifier expected"));
    return ParserResult<Statement>(); // Error
  }

  std::unique_ptr<Identifier> id(factory_.NewIdentifier(
      boost::get<std::string>(token_.GetValue())));

  Advance(); // Consume the token
  ValidToken(); // Avance until find a valid token

  if (token_ != TokenKind::ASSIGN) {
    ErrorMsg(boost::format("assign expected"));
    return ParserResult<Statement>(); // Error
  }

  TokenKind kind = token_.GetKind();

  Advance();
  ValidToken(); // Avance until find a valid token
  ParserResult<Expression> exp = ParserArithExp();
  return ParserResult<Statement>(factory_.NewAssignmentStatement(
      kind, std::move(id), std::move(exp.MoveAstNode())));
}

ParserResult<Expression> Parser::ParserArithExp() {
  ParserResult<Expression> rexp;
  ParserResult<Expression> lexp = ParserTerm();

  if (!lexp) {
    return ParserResult<Expression>(); // Error
  }

  if (token_.IsAny(TokenKind::ADD, TokenKind::SUB)) {
    TokenKind token_kind = token_.GetKind();
    Advance();
    rexp = std::move(ParserArithExp());

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
  ParserResult<Expression> lexp = ParserUnaryExp();

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

ParserResult<Expression> Parser::ParserUnaryExp() {
  if (token_.IsAny(TokenKind::ADD, TokenKind::SUB)) {
    TokenKind token_kind = token_.GetKind();
    Advance(); // Consume the token
    ParserResult<Expression> exp = ParserPostExp();
    return ParserResult<Expression>(factory_.NewUnaryOperation(
          token_kind, std::move(exp.MoveAstNode())));
  }

  return ParserPostExp();
}

ParserResult<Expression> Parser::ParserPostExp() {
  ParserResult<Expression> exp = ParserPrimaryExp();

  while (token_.IsAny(TokenKind::LBRACKET, TokenKind::ARROW)) {
    if (token_ == TokenKind::LBRACKET) {
      Advance();
      ValidToken();
      ParserResult<Expression> index_exp = ParserArithExp();
      ValidToken();
      if (token_ != TokenKind::RBRACKET) {
        ErrorMsg(boost::format("Expected ']' in the end of expression"));
        return ParserResult<Expression>(); // Error
      }
      Advance();

      exp = std::move(ParserResult<Expression>(
        factory_.NewArray(
            std::move(exp.MoveAstNode()),
            std::move(index_exp.MoveAstNode())
          )
        )
      );
    } else if (token_ == TokenKind::ARROW) {
      Advance();
      ValidToken();
      if (token_ != TokenKind::IDENTIFIER) {
        ErrorMsg(boost::format("Expected identifier"));
        return ParserResult<Expression>(); // Error
      }
        ParserResult<Identifier> id = ParserResult<Identifier>(
            factory_.NewIdentifier(boost::get<std::string>(token_.GetValue())));
        Advance(); // Consume the token

        exp = std::move(ParserResult<Expression>(
        factory_.NewAttribute(
            std::move(exp.MoveAstNode()),
            std::move(id.MoveAstNode())
          )
        )
      );
    }
  }

  return exp;
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
    ParserResult<Expression> res = ParserArithExp();
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
