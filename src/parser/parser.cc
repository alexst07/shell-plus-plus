#include "parser.h"

#include <sstream>

namespace setti {
namespace internal {

ParserResult<Statement> Parser::ParserIfStmt() {
  if (token_ != TokenKind::KW_IF) {
    ErrorMsg(boost::format("expected if statement"));
    return ParserResult<Statement>(); // Error
  }

  Advance();
  ValidToken();

  ParserResult<Expression> exp(ParserOrExp());

  ValidToken();

  ParserResult<Statement> then_block(ParserBlock());

  if (token_ == TokenKind::KW_ELSE) {
    ParserResult<Statement> else_block;

    Advance();

    if (ValidToken() == TokenKind::KW_IF) {
      else_block = std::move(ParserIfStmt());
    } else {
      else_block = std::move(ParserBlock());
    }

    return ParserResult<Statement>(factory_.NewIfStatement(
      exp.MoveAstNode(), then_block.MoveAstNode(), else_block.MoveAstNode()));
  }

  return ParserResult<Statement>(factory_.NewIfStatement(
      exp.MoveAstNode(), then_block.MoveAstNode(), nullptr));
}

ParserResult<Statement> Parser::ParserSwitchStmt() {
  if (token_ != TokenKind::KW_SWITCH) {
    ErrorMsg(boost::format("expected switch token, got %1%")% TokenValueStr());
    return ParserResult<Statement>(); // Error
  }

  ParserResult<Expression> exp;

  Advance();

  if (ValidToken() == TokenKind::LBRACE) {
    exp = std::move(nullptr);
  } else {
    exp = std::move(ParserOrExp());
  }

  ValidToken();

  ParserResult<Statement> block(ParserBlock());

  return ParserResult<Statement>(factory_.NewSwitchStatement(exp.MoveAstNode(),
      block.MoveAstNode()));
}

ParserResult<Statement> Parser::ParserCaseStmt() {
  if (token_ != TokenKind::KW_CASE) {
    ErrorMsg(boost::format("expected case token, got %1%")% TokenValueStr());
    return ParserResult<Statement>(); // Error
  }

  Advance();
  ValidToken();

  ParserResult<Expression> exp(ParserOrExp());

  if (ValidToken() != TokenKind::COLON) {
    ErrorMsg(boost::format("expected ':' token, got %1%")% TokenValueStr());
    return ParserResult<Statement>(); // Error
  }

  Advance();
  ValidToken();

  return ParserResult<Statement>(factory_.NewCaseStatement(exp.MoveAstNode()));
}

ParserResult<Statement> Parser::ParserBlock() {
  if (token_ != TokenKind::LBRACE) {
    ErrorMsg(boost::format("expected { token, got %1%")% TokenValueStr());
      return ParserResult<Statement>(); // Error
  }

  Advance();
  ValidToken();

  ParserResult<StatementList> stmt_list(ParserStmtList());

  if (ValidToken() != TokenKind::RBRACE) {
    ErrorMsg(boost::format("expected } token, got %1%")% TokenValueStr());
      return ParserResult<Statement>(); // Error
  }

  Advance();
  ValidToken();

  return ParserResult<Statement>(factory_.NewBlock(stmt_list.MoveAstNode()));
}

ParserResult<StatementList> Parser::ParserStmtList() {
  std::vector<std::unique_ptr<Statement>> stmt_list;

  while (token_.IsNot(TokenKind::EOS, TokenKind::RBRACE)) {
    ValidToken();
    ParserResult<Statement> stmt = ParserStmt();
    stmt_list.push_back(stmt.MoveAstNode());

    // uses new line char as end of statement, and advance until valid
    // a valid token
    if (token_ == TokenKind::NWL) {
      ValidToken();
    } else if (token_.IsAny(TokenKind::EOS, TokenKind::RBRACE)) {
      // end of file and end of block are a valid end for statement
      break;
    } else {
      continue;
    }
  }

  return ParserResult<StatementList>(factory_.NewStatementList(
      std::move(stmt_list)));
}

ParserResult<Statement> Parser::ParserWhileStmt() {
  if (token_ != TokenKind::KW_WHILE) {
    ErrorMsg(boost::format("expected while statement"));
    return ParserResult<Statement>(); // Error
  }

  Advance();
  ValidToken();

  ParserResult<Expression> exp(ParserOrExp());

  ValidToken();

  ParserResult<Statement> block(ParserBlock());

  return ParserResult<Statement>(factory_.NewWhileStatement(
      exp.MoveAstNode(), block.MoveAstNode()));
}

ParserResult<Statement> Parser::ParserStmt() {
  if (token_ == TokenKind::KW_IF) {
    return ParserIfStmt();
  } else if (token_ == TokenKind::KW_WHILE) {
    return ParserWhileStmt();
  } else if (token_ == TokenKind::KW_BREAK) {
    return ParserBreakStmt();
  } else if (token_ == TokenKind::KW_CASE) {
    return ParserCaseStmt();
  } else if (token_ == TokenKind::KW_DEFAULT) {
    return ParserDefaultStmt();
  } else if (token_ == TokenKind::KW_SWITCH) {
    return ParserSwitchStmt();
  } else {
    return ParserSimpleStmt();
  }
}

ParserResult<Statement> Parser::ParserBreakStmt() {
  if (token_ != TokenKind::KW_BREAK) {
    ErrorMsg(boost::format("expected break token, got %1%")% TokenValueStr());
      return ParserResult<Statement>(); // Error
  }

  Advance();

  return ParserResult<Statement>(factory_.NewBreakStatement());
}

ParserResult<Statement> Parser::ParserDefaultStmt() {
  if (token_ != TokenKind::KW_DEFAULT) {
    ErrorMsg(boost::format("expected default token, got %1%")% TokenValueStr());
      return ParserResult<Statement>(); // Error
  }

  Advance();

  if (token_ != TokenKind::COLON) {
    ErrorMsg(boost::format("expected ':' token, got %1%")% TokenValueStr());
      return ParserResult<Statement>(); // Error
  }

  Advance();

  return ParserResult<Statement>(factory_.NewDefaultStatement());
}

ParserResult<Statement> Parser::ParserSimpleStmt() {
  enum Type {kErro, kAssign, kExpStm};
  Type type = kErro;
  std::vector<std::unique_ptr<Expression>> vec_list;
  ParserResult<ExpressionList> rexp_list;
  size_t num_comma = 0;
  TokenKind kind;

  do {
    ParserResult<Expression> exp = ParserPostExp();
    vec_list.push_back(exp.MoveAstNode());

    if (token_.Is(TokenKind::COMMA)) {num_comma++;}
  } while (CheckComma());

  if ((num_comma > 0) && token_.IsNot(TokenKind::ASSIGN)) {
    ErrorMsg(boost::format("assign expected"));
    return ParserResult<Statement>(); // Error
  }

  type = kExpStm;

  if (Token::IsAssignToken(token_.GetKind())) {
    type = Type::kAssign;
    kind = token_.GetKind();

    Advance(); // consume assign token
    ValidToken();

    rexp_list = ParserExpList();
  }

  switch (type) {
    case Type::kAssign:
      return ParserResult<Statement>(factory_.NewAssignmentStatement(
          kind, factory_.NewExpressionList(std::move(vec_list)),
          rexp_list.MoveAstNode()));
      break;

    case Type::kExpStm:
      return ParserResult<Statement>(factory_.NewExpressionStatement(
          std::move(vec_list[0])));
      break;

    default:
      ErrorMsg(boost::format("not a statement"));
      return ParserResult<Statement>(); // Error
  }
}

ParserResult<ExpressionList> Parser::ParserExpList() {
  std::vector<std::unique_ptr<Expression>> vec_exp;

  do {
    ParserResult<Expression> exp = ParserOrExp();
    vec_exp.push_back(exp.MoveAstNode());
  } while (CheckComma());

  return ParserResult<ExpressionList>(factory_.NewExpressionList(
      std::move(vec_exp)));
}

ParserResult<Expression> Parser::ParserOrExp() {
  ParserResult<Expression> rexp;
  ParserResult<Expression> lexp = ParserAndExp();

  if (!lexp) {
    return ParserResult<Expression>(); // Error
  }

  if (token_.Is(TokenKind::OR)) {
    TokenKind token_kind = token_.GetKind();
    Advance();
    ValidToken();

    rexp = std::move(ParserOrExp());

    if (rexp) {
      return ParserResult<Expression>(factory_.NewBinaryOperation(
          token_kind, lexp.MoveAstNode(), rexp.MoveAstNode()));
    }
  }

  return lexp;
}

ParserResult<Expression> Parser::ParserAndExp() {
  ParserResult<Expression> rexp;
  ParserResult<Expression> lexp = ParserNotExp();

  if (!lexp) {
    return ParserResult<Expression>(); // Error
  }

  if (token_.Is(TokenKind::AND)) {
    TokenKind token_kind = token_.GetKind();
    Advance();
    ValidToken();

    rexp = std::move(ParserAndExp());

    if (rexp) {
      return ParserResult<Expression>(factory_.NewBinaryOperation(
          token_kind, lexp.MoveAstNode(), rexp.MoveAstNode()));
    }
  }

  return lexp;
}

ParserResult<Expression> Parser::ParserNotExp() {
  if (token_.Is(TokenKind::NOT)) {
    TokenKind token_kind = token_.GetKind();
    Advance(); // Consume the token
    ValidToken();

    ParserResult<Expression> exp = ParserComparisonExp();
    return ParserResult<Expression>(factory_.NewUnaryOperation(
          token_kind, exp.MoveAstNode()));
  }

  return ParserComparisonExp();
}

ParserResult<Expression> Parser::ParserComparisonExp() {
  ParserResult<Expression> rexp;
  ParserResult<Expression> lexp = ParserBitOrExp();

  if (!lexp) {
    return ParserResult<Expression>(); // Error
  }

  if (Token::IsComparisonToken(token_.GetKind())) {
    TokenKind token_kind = token_.GetKind();
    Advance();
    ValidToken();

    rexp = std::move(ParserComparisonExp());

    if (rexp) {
      return ParserResult<Expression>(factory_.NewBinaryOperation(
          token_kind, lexp.MoveAstNode(), rexp.MoveAstNode()));
    }
  }

  return lexp;
}

ParserResult<Expression> Parser::ParserBitOrExp() {
  ParserResult<Expression> rexp;
  ParserResult<Expression> lexp = ParserBitXorExp();

  if (!lexp) {
    return ParserResult<Expression>(); // Error
  }

  if (token_.Is(TokenKind::BIT_OR)) {
    TokenKind token_kind = token_.GetKind();
    Advance();
    ValidToken();

    rexp = std::move(ParserBitOrExp());

    if (rexp) {
      return ParserResult<Expression>(factory_.NewBinaryOperation(
          token_kind, lexp.MoveAstNode(), rexp.MoveAstNode()));
    }
  }

  return lexp;
}

ParserResult<Expression> Parser::ParserBitXorExp() {
  ParserResult<Expression> rexp;
  ParserResult<Expression> lexp = ParserBitAndExp();

  if (!lexp) {
    return ParserResult<Expression>(); // Error
  }

  if (token_.Is(TokenKind::BIT_XOR)) {
    TokenKind token_kind = token_.GetKind();
    Advance();
    ValidToken();

    rexp = std::move(ParserBitXorExp());

    if (rexp) {
      return ParserResult<Expression>(factory_.NewBinaryOperation(
          token_kind, lexp.MoveAstNode(), rexp.MoveAstNode()));
    }
  }

  return lexp;
}

ParserResult<Expression> Parser::ParserBitAndExp() {
  ParserResult<Expression> rexp;
  ParserResult<Expression> lexp = ParserShiftExp();

  if (!lexp) {
    return ParserResult<Expression>(); // Error
  }

  if (token_.Is(TokenKind::BIT_AND)) {
    TokenKind token_kind = token_.GetKind();
    Advance();
    ValidToken();

    rexp = std::move(ParserBitAndExp());

    if (rexp) {
      return ParserResult<Expression>(factory_.NewBinaryOperation(
          token_kind, lexp.MoveAstNode(), rexp.MoveAstNode()));
    }
  }

  return lexp;
}

ParserResult<Expression> Parser::ParserShiftExp() {
  ParserResult<Expression> rexp;
  ParserResult<Expression> lexp = ParserArithExp();

  if (!lexp) {
    return ParserResult<Expression>(); // Error
  }

  if (token_.IsAny(TokenKind::SHL, TokenKind::SAR)) {
    TokenKind token_kind = token_.GetKind();
    Advance();
    ValidToken();

    rexp = std::move(ParserShiftExp());

    if (rexp) {
      return ParserResult<Expression>(factory_.NewBinaryOperation(
          token_kind, lexp.MoveAstNode(), rexp.MoveAstNode()));
    }
  }

  return lexp;
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
    ValidToken();

    rexp = std::move(ParserArithExp());

    if (rexp) {
      return ParserResult<Expression>(factory_.NewBinaryOperation(
          token_kind, lexp.MoveAstNode(), rexp.MoveAstNode()));
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
    ValidToken();

    rexp = std::move(ParserTerm());

    if (rexp) {
      return ParserResult<Expression>(factory_.NewBinaryOperation(
          token_kind, lexp.MoveAstNode(),rexp.MoveAstNode()));
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
          token_kind, exp.MoveAstNode()));
  }

  return ParserPostExp();
}

ParserResult<Expression> Parser::ParserPostExp() {
  ParserResult<Expression> exp = ParserPrimaryExp();

  while (token_.IsAny(TokenKind::LBRACKET, TokenKind::ARROW,
         TokenKind::LPAREN)) {
    if (token_ == TokenKind::LBRACKET) {
      // parser array
      Advance();
      ValidToken();
      ParserResult<Expression> index_exp(ParserArithExp());

      if (ValidToken().IsNot(TokenKind::RBRACKET)) {
        ErrorMsg(boost::format("Expected ']' in the end of expression"));
        return ParserResult<Expression>(); // Error
      }
      Advance();

      exp = factory_.NewArray(exp.MoveAstNode(),index_exp.MoveAstNode());
    } else if (token_ == TokenKind::ARROW) {
      // parser attributes
      Advance();
      if (ValidToken().IsNot(TokenKind::IDENTIFIER)) {
        ErrorMsg(boost::format("Expected identifier"));
        return ParserResult<Expression>(); // Error
      }

      ParserResult<Identifier> id(factory_.NewIdentifier(
          boost::get<std::string>(token_.GetValue())));
      Advance(); // Consume the token

      exp = factory_.NewAttribute(exp.MoveAstNode(), id.MoveAstNode());
    } else if (token_ == TokenKind::LPAREN) {
      // parser function call
      Advance();

      std::vector<std::unique_ptr<Expression>> exp_list;

      if (ValidToken().Is(TokenKind::RPAREN)) {
        // empty expression list
        exp = factory_.NewFunctionCall(
            exp.MoveAstNode(), factory_.NewExpressionList(
                std::move(exp_list)));
      } else {
        // Parser expression list separted by (,) comma
        auto res_exp_list = ParserExpList();
        exp = factory_.NewFunctionCall(exp.MoveAstNode(),
                                       res_exp_list.MoveAstNode());

        if (ValidToken().IsNot(TokenKind::RPAREN)) {
          ErrorMsg(boost::format("Expected close right paren"));
          return ParserResult<Expression>(); // Error
        }
      } // if token_ == TokenKind::RPAREN

      Advance(); // advance rparen ')'
    } // if token_ == TokenKind::LPAREN
  } // while

  return exp;
}

ParserResult<Expression> Parser::ParserPrimaryExp() {
  Token token(ValidToken());
  if (token == TokenKind::IDENTIFIER) {
    ParserResult<Expression> res(
        factory_.NewIdentifier(boost::get<std::string>(token.GetValue())));
    Advance(); // Consume the token
    return res;
  } else if (token == TokenKind::LPAREN) {
    Advance(); // consume the token '('
    ParserResult<Expression> res(ParserAndExp());
    if (ValidToken() != TokenKind::RPAREN) {
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
    ErrorMsg(boost::format("primary expression expected, got %1%")
        % Token::TokenValueToStr(token.GetValue()));
    return ParserResult<Expression>(); // Error
  }
}

}
}
