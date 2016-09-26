#include "parser.h"

#include <sstream>

namespace setti {
namespace internal {

ParserResult<StatementList> Parser::ParserStmtList() {
  std::vector<std::unique_ptr<Statement>> stmt_list;

  while (token_ != TokenKind::EOS) {
    ValidToken();
    ParserResult<Statement> stmt = ParserAssignStmt();
    stmt_list.push_back(stmt.MoveAstNode());

    // uses new line char as end of statement, and advance until valid
    // a valid token
    if (token_ == TokenKind::NWL) {
      ValidToken();
    } else if (token_ == TokenKind::EOS) {
      // end of file is a valid end for statement
      break;
    } else {
      ErrorMsg(boost::format("end of stmt expected"));
      return ParserResult<StatementList>(); // Error
    }
  }

  return ParserResult<StatementList>(factory_.NewStatementList(
      std::move(stmt_list)));
}

ParserResult<Statement> Parser::ParserAssignStmt() {
  auto lexp_list = ParserExpList();
  exp = factory_.NewFunctionCall(exp.MoveAstNode(),
                                       res_exp_list.MoveAstNode());

  std::unique_ptr<Identifier> id(factory_.NewIdentifier(
      boost::get<std::string>(token_.GetValue())));

  Advance(); // Consume the token

  if (ValidToken().IsNot(TokenKind::ASSIGN)) {
    ErrorMsg(boost::format("assign expected"));
    return ParserResult<Statement>(); // Error
  }

  TokenKind kind = token_.GetKind();

  Advance();
  ValidToken(); // Avance until find a valid token
  ParserResult<Expression> exp(ParserArithExp());
  return ParserResult<Statement>(factory_.NewAssignmentStatement(
      kind, std::move(id), exp.MoveAstNode()));
}

ParserResult<ExpressionList> Parser::ParserExpList() {
  std::vector<std::unique_ptr<Expression>> vec_exp;

  auto check_token = [&]() -> bool {
    if (ValidToken().Is(TokenKind::COMMA)) {
      Advance();
      return true;
    } else {
      return false;
    }
  };

  do {
    ParserResult<Expression> exp = ParserArithExp();
    vec_exp.push_back(exp.MoveAstNode());
  } while (check_token());

  return ParserResult<ExpressionList>(factory_.NewExpressionList(
      std::move(vec_exp)));
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
    ParserResult<Expression> res(ParserArithExp());
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
