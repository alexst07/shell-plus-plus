#include "parser.h"

#include <sstream>

namespace setti {
namespace internal {

ParserResult<Statement> Parser::ParserStmtDecl() {
  if (token_ == TokenKind::KW_FUNC) {
    ParserResult<Declaration> func(ParserFunctionDeclaration(false));
    return ParserResult<Statement>(func.MoveAstNode<Statement>());
  }

  return ParserResult<Statement>(); // error
}

bool Parser::IsStmtDecl() {
  return token_.IsAny(TokenKind::KW_FUNC);
}

std::tuple<std::vector<std::unique_ptr<FunctionParam>>, bool>
Parser::ParserParamsList() {
  std::vector<std::unique_ptr<FunctionParam>> vec_params;
  while (true) {
    if (token_ != TokenKind::IDENTIFIER) {
      ErrorMsg(boost::format("expected identifier"));
      return std::tuple<std::vector<std::unique_ptr<FunctionParam>>, bool>(
          std::move(vec_params), false); // Error
    }

    std::unique_ptr<Identifier> id(factory_.NewIdentifier(
        boost::get<std::string>(token_.GetValue())));

    Advance();
    ValidToken();

    bool variadic = false;

    if (token_ == TokenKind::ELLIPSIS) {
      variadic = true;
      Advance();
      ValidToken();
    }

    std::unique_ptr<FunctionParam> param(factory_.NewFunctionParam(
        std::move(id), variadic));
    vec_params.push_back(std::move(param));

    // if the token is comma (,) goes to next parameter
    // if not, break the loop to return
    if (token_ == TokenKind::COMMA) {
      Advance();
      ValidToken();
    } else {
      break;
    }
  }

  return std::tuple<std::vector<std::unique_ptr<FunctionParam>>, bool>(
      std::move(vec_params), true); // Error
}

ParserResult<Declaration> Parser::ParserFunctionDeclaration(bool lambda) {
  if (token_ != TokenKind::KW_FUNC) {
    ErrorMsg(boost::format("expected function"));
    return ParserResult<Declaration>(); // Error
  }

  Advance();
  ValidToken();

  std::unique_ptr<Identifier> id;

  // If is a lambda function, the function doesn't have identifier name
  if (!lambda) {
    if (token_ != TokenKind::IDENTIFIER) {
      ErrorMsg(boost::format("expected identifier"));
      return ParserResult<Declaration>(); // Error
    }

    id = std::move(factory_.NewIdentifier(boost::get<std::string>(
        token_.GetValue()), std::move(nullptr)));

    Advance();
    ValidToken();
  }

  if (token_ != TokenKind::LPAREN) {
    ErrorMsg(boost::format("expected token '(' got %1%")% TokenValueStr());
    return ParserResult<Declaration>(); // Error
  }

  Advance();
  ValidToken();

  std::vector<std::unique_ptr<FunctionParam>> func_params;

  if (token_ == TokenKind::RPAREN) {
    Advance();
    ValidToken();
  } else {
    bool ok = true;
    std::tie(func_params, ok) = ParserParamsList();
    if (token_ != TokenKind::RPAREN) {
      ErrorMsg(boost::format("expected token ')'"));
      return ParserResult<Declaration>(); // Error
    }

    if (!ok) {
      return ParserResult<Declaration>(); // Error
    }

    Advance();
    ValidToken();
  }

  std::unique_ptr<Block> block(ParserBlock().MoveAstNode<Block>());

  return ParserResult<Declaration>(factory_.NewFunctionDeclaration(
      std::move(func_params), std::move(id), std::move(block)));
}

ParserResult<Statement> Parser::ParserForInStmt() {
  if (token_ != TokenKind::KW_FOR) {
    ErrorMsg(boost::format("expected for statement"));
    return ParserResult<Statement>(); // Error
  }

  Advance();
  ValidToken();
  ParserResult<ExpressionList> exp_list(ParserPostExpList());

  if (token_ != TokenKind::KW_IN) {
    ErrorMsg(boost::format("expected in operator"));
    return ParserResult<Statement>(); // Error
  }

  Advance();
  ValidToken();
  ParserResult<ExpressionList> test_list(ParserExpList());

  ValidToken();
  ParserResult<Statement> block(ParserBlock());

  if (!block || !test_list || !exp_list) {
    return ParserResult<Statement>();
  }

  return ParserResult<Statement>(factory_.NewForInStatement(
      exp_list.MoveAstNode(), test_list.MoveAstNode(), block.MoveAstNode()));
}

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

  if (!then_block) {
    return ParserResult<Statement>();
  }

  if (token_ == TokenKind::KW_ELSE) {
    ParserResult<Statement> else_block;

    Advance();

    if (ValidToken() == TokenKind::KW_IF) {
      else_block = std::move(ParserIfStmt());
    } else {
      else_block = std::move(ParserBlock());
    }

    if (!else_block) {
      return ParserResult<Statement>();
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

  if (!block || !exp) {
    return ParserResult<Statement>();
  }

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

  if (!stmt_list) {
    return ParserResult<Statement>();
  }

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

  // Check RBRACE because it is the end of block, and as the
  // stmt list is inside the block, it has to check token
  // RBRACE(}) to know if it is in the end of the block
  while (token_.IsNot(TokenKind::EOS, TokenKind::RBRACE)) {
    ValidToken();
    ParserResult<Statement> stmt(ParserStmt());
    if (!stmt) {
      return ParserResult<StatementList>();
    }

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
  } else if (token_ == TokenKind::KW_FOR) {
    return ParserForInStmt();
  } else if (token_ == TokenKind::LBRACE) {
    return ParserBlock();
  } else if (IsStmtDecl()) {
    return ParserStmtDecl();
  } else if (MatchLangStmt()) {
    return ParserSimpleStmt();
  } else {
    return ParserCmdFull();
  }
}

ParserResult<Statement> Parser::ParserCmdFull() {
  ParserResult<Statement> cmd = ParserCmdAndOr();
  bool background_exec = false;

  if (!cmd) {
    return ParserResult<Statement>(); // Error
  }

  if (token_ == TokenKind::BIT_AND) {
    background_exec = true;
    Advance();
  } else if (token_ == TokenKind::SEMI_COLON) {
    Advance();
  } else if (!TokenEndFullCmd()) {
    ErrorMsg(boost::format("unexpected token in the end of command"));
    return ParserResult<Statement>(); // Error
  }

  return ParserResult<Statement>(factory_.NewCmdFull(
      cmd.MoveAstNode<Cmd>(), background_exec));
}

ParserResult<Statement> Parser::ParserCmdAndOr() {
  ParserResult<Statement> rcmd;
  ParserResult<Statement> lcmd = ParserCmdPipe();

  if (!lcmd) {
    return ParserResult<Statement>(); // Error
  }

  while (token_.IsAny(TokenKind::OR, TokenKind::AND)) {
    TokenKind token_kind = token_.GetKind();
    Advance();
    ValidToken();

    rcmd = std::move(ParserCmdPipe());

    if (rcmd) {
      lcmd = std::move(factory_.NewCmdAndOr(
          token_kind, lcmd.MoveAstNode<Cmd>(), rcmd.MoveAstNode<Cmd>()));
    }
  }

  return lcmd;
}

ParserResult<Statement> Parser::ParserCmdPipe() {
  ParserResult<Statement> rstmt;
  ParserResult<Statement> lstmt = ParserIoRedirectCmdList();

  if (!lstmt) {
    return ParserResult<Statement>(); // Error
  }

  while (token_.Is(TokenKind::BIT_OR)) {
    TokenKind token_kind = token_.GetKind();
    Advance();
    ValidToken();

    rstmt = std::move(ParserIoRedirectCmdList());

    if (rstmt) {
      lstmt = std::move(factory_.NewCmdPipeSequence(
          lstmt.MoveAstNode<Cmd>(), rstmt.MoveAstNode<Cmd>()));
    }
  }

  return lstmt;
}

bool Parser::IsIoRedirect() {
  if (CmdValidInt() || IsIOToken(token_)) {
    return true;
  }

  return false;
}

ParserResult<Statement> Parser::ParserIoRedirectCmdList() {
  ParserResult<Statement> simple_cmd(ParserSimpleCmd());

  if (!simple_cmd) {
    return ParserResult<Statement>(); // Error
  }

  std::vector<std::unique_ptr<CmdIoRedirect>> vec_io;

  // Check if there are some io redirect on command
  while (IsIoRedirect()) {
    // Gets each io redirect
    std::unique_ptr<CmdIoRedirect> io(std::move(ParserIoRedirectCmd()));
    vec_io.push_back(std::move(io));
  }

  if (vec_io.empty()) {
    return simple_cmd;
  }

  // Cast from statment to command
  std::unique_ptr<Cmd> cmdptr(simple_cmd.MoveAstNode<Cmd>());

  return ParserResult<Statement>(factory_.NewCmdIoRedirectList(
      std::move(cmdptr), std::move(vec_io)));
}

std::unique_ptr<CmdIoRedirect> Parser::ParserIoRedirectCmd() {
  ParserResult<Expression> integer(nullptr);
  bool all = false; // all output interfaces

  // Check if is an int before io redirect as 2> or 2>> for example
  if (CmdValidInt()) {
    integer = std::move(factory_.NewLiteral(
        token_.GetValue(), Literal::kInteger));
    Advance();
  } else if (token_ == TokenKind::BIT_AND && IsIOToken(PeekAhead())) {
    // Check if the token is & follows by io token: &> or &>>
    all = true;
    Advance();
  }

  std::vector<std::unique_ptr<AstNode>> pieces;
  TokenKind kind = token_.GetKind();
  Advance();

  // All tokens that doesn't mean any special token to command is
  // get as pieces of file path
  while (!IsCmdStopPoint()) {
    // Parser an expression inside the path
    // ex: cmd_any > f${v[0]}.any
    if (token_ == TokenKind::DOLLAR_LBRACE) {
      ParserResult<Expression> exp(ParserExpCmd());
      pieces.push_back(std::move(exp.MoveAstNode()));
      continue;
    }

    // Puts piece of the file path on a vector, this vector will be
    // the path of file
    auto piece = factory_.NewCmdPiece(token_);
    pieces.push_back(std::move(piece));
    Advance();
  }

  std::unique_ptr<FilePathCmd> path(
      factory_.NewFilePathCmd(std::move(pieces)));

  std::unique_ptr<Literal> intptr(integer.MoveAstNode<Literal>());

  return factory_.NewCmdIoRedirect(std::move(intptr), std::move(path), kind,
                                   all);

}

ParserResult<Statement> Parser::ParserSimpleCmd() {
  std::vector<std::unique_ptr<AstNode>> pieces;

  ValidToken();
  int num_pieces = 0;
  while (!IsCmdStopPoint()) {
    // Count if the command has some pieces
    num_pieces++;

    // Parser an expression inside command
    // ex: cmd -e ${v[0] + 1} -d
    if (token_ == TokenKind::DOLLAR_LBRACE) {
      ParserResult<Expression> exp(ParserExpCmd());
      pieces.push_back(std::move(exp.MoveAstNode()));
      continue;
    }

    // Puts piece of the command on a vector, this vector will be the
    // entire commmand
    auto piece = factory_.NewCmdPiece(token_);
    pieces.push_back(std::move(piece));
    Advance();
  }

  // if the command is empty there is an error
  if (num_pieces == 0) {
    ErrorMsg(boost::format("Empty command, token %1% unexpected")%
             TokenValueStr());
    return ParserResult<Statement>(); // Error
  }

  return ParserResult<Statement>(factory_.NewSimpleCmd(std::move(pieces)));
}

ParserResult<Expression> Parser::ParserExpCmd() {
  // Parser an expression inside command
  // ex: any_cmd -e ${v[0] + 1} -d
  if (token_ != TokenKind::DOLLAR_LBRACE) {
    ErrorMsg(boost::format("expected ${ token"));
    return ParserResult<Expression>(); // Error
  }

  Advance();
  if (token_ == TokenKind::NWL) {
    ErrorMsg(boost::format("New line not allowed"));
    return ParserResult<Expression>(); // Error
  }

  ParserResult<Expression> exp(ParserOrExp());

  if (token_ != TokenKind::RBRACE) {
    ErrorMsg(boost::format("token '}' expected"));
    return ParserResult<Expression>(); // Error
  }
  Advance();

  return exp;
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
      // if there is no assing token, so it is a statement expression
      // function call is the only expression accept, this check is
      // done on semantic analysis
      return ParserResult<Statement>(factory_.NewExpressionStatement(
          std::move(vec_list[0])));
      break;

    default:
      ErrorMsg(boost::format("not a statement"));
      return ParserResult<Statement>(); // Error
  }
}

ParserResult<ExpressionList> Parser::ParserPostExpList() {
  std::vector<std::unique_ptr<Expression>> vec_list;

  do {
    ParserResult<Expression> exp(ParserPostExp());
    vec_list.push_back(exp.MoveAstNode());
  } while (CheckComma());

  return ParserResult<ExpressionList>(factory_.NewExpressionList(
      std::move(vec_list)));
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

  while (token_.Is(TokenKind::OR)) {
    TokenKind token_kind = token_.GetKind();
    Advance();
    ValidToken();

    rexp = std::move(ParserAndExp());

    if (rexp) {
      lexp = std::move(factory_.NewBinaryOperation(
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

  while (token_.Is(TokenKind::AND)) {
    TokenKind token_kind = token_.GetKind();
    Advance();
    ValidToken();

    rexp = std::move(ParserNotExp());

    if (rexp) {
      lexp = std::move(factory_.NewBinaryOperation(
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

  while (Token::IsComparisonToken(token_.GetKind())) {
    TokenKind token_kind = token_.GetKind();
    Advance();
    ValidToken();

    rexp = std::move(ParserBitOrExp());

    if (rexp) {
      lexp = std::move(factory_.NewBinaryOperation(
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

  while (token_.Is(TokenKind::BIT_OR)) {
    TokenKind token_kind = token_.GetKind();
    Advance();
    ValidToken();

    rexp = std::move(ParserBitXorExp());

    if (rexp) {
      lexp = std::move(factory_.NewBinaryOperation(
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

  while (token_.Is(TokenKind::BIT_XOR)) {
    TokenKind token_kind = token_.GetKind();
    Advance();
    ValidToken();

    rexp = std::move(ParserBitAndExp());

    if (rexp) {
      lexp = std::move(factory_.NewBinaryOperation(
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

  while (token_.Is(TokenKind::BIT_AND)) {
    TokenKind token_kind = token_.GetKind();
    Advance();
    ValidToken();

    rexp = std::move(ParserShiftExp());

    if (rexp) {
      lexp = std::move(factory_.NewBinaryOperation(
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

  while (token_.IsAny(TokenKind::SHL, TokenKind::SAR)) {
    TokenKind token_kind = token_.GetKind();
    Advance();
    ValidToken();

    rexp = std::move(ParserArithExp());

    if (rexp) {
      lexp = std::move(factory_.NewBinaryOperation(
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

  while (token_.IsAny(TokenKind::ADD, TokenKind::SUB)) {
    TokenKind token_kind = token_.GetKind();
    Advance();
    ValidToken();

    rexp = std::move(ParserTerm());

    if (rexp) {
      lexp = std::move(factory_.NewBinaryOperation(
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

  while (token_.IsAny(TokenKind::MUL, TokenKind::DIV)) {
    TokenKind token_kind = token_.GetKind();
    Advance();
    ValidToken();

    rexp = std::move(ParserUnaryExp());

    if (rexp) {
      lexp = std::move(factory_.NewBinaryOperation(
          token_kind, lexp.MoveAstNode(), rexp.MoveAstNode()));
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

ParserResult<Expression> Parser::ParserScopeIdentifier() {
  std::unique_ptr<Identifier> id(
      factory_.NewIdentifier(boost::get<std::string>(token_.GetValue())));

  Advance(); // Consume the id token

  while (token_ == TokenKind::SCOPE) {
    std::unique_ptr<PackageScope> scope(factory_.NewPackageScope(
        std::move(id)));
    Advance();

    if (token_ != TokenKind::IDENTIFIER) {
      ErrorMsg(boost::format("Expected identifier after scope operator"));
      return ParserResult<Expression>(); // Error
    }

    id = std::move(factory_.NewIdentifier(boost::get<std::string>(
        token_.GetValue()), std::move(scope)));
    Advance();
  }

  ParserResult<Expression> res(std::move(id));
  return res;
}

ParserResult<Expression> Parser::ParserPrimaryExp() {
  if (token_ == TokenKind::IDENTIFIER) {
    // parser scope id: scope1::scope2::id
    return ParserScopeIdentifier();
  } else if (token_ == TokenKind::DOLLAR_LPAREN) {
    Advance(); // consume the token '$('

    std::unique_ptr<CmdFull> cmd(ParserCmdFull().MoveAstNode<CmdFull>());
    Advance(); // consume the token ')'

    ParserResult<Expression> res(factory_.NewCmdExpression(std::move(cmd)));
    return res;
  } else if (token_ == TokenKind::LPAREN) {
    Advance(); // consume the token '('
    ParserResult<Expression> res(ParserOrExp());

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
