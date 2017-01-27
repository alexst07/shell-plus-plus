// Copyright 2016 Alex Silva Torres
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "parser.h"

#include <sstream>

namespace seti {
namespace internal {

ParserResult<Statement> Parser::ParserImportStmt() {
  Advance();
  ValidToken();

  if (token_ == TokenKind::IDENTIFIER) {
    return ParserImportIdStmt();
  } else if (token_ == TokenKind::STRING_LITERAL) {
    return ParserImportPathStmt();
  } else {
    ErrorMsg(boost::format("import statement not valid"));
    return ParserResult<Statement>(); // Error
  }
}

ParserResult<Statement> Parser::ParserImportIdStmt() {
  std::unique_ptr<Identifier> path(factory_.NewIdentifier(
                                   boost::get<std::string>(token_.GetValue())));

  Advance();

  std::unique_ptr<Identifier> id;

  if (token_ == TokenKind::KW_AS) {
    Advance();
    ValidToken();

    if (token_ != TokenKind::IDENTIFIER) {
      ErrorMsg(boost::format("expected identifier"));
      return ParserResult<Statement>(); // Error
    }

    id = std::move(factory_.NewIdentifier(
                   boost::get<std::string>(token_.GetValue())));

    Advance();
  }

  return ParserResult<Statement>(
        factory_.NewImportStatement(std::move(path), std::move(id)));
}

ParserResult<Statement> Parser::ParserImportPathStmt() {
  std::unique_ptr<Literal> path(factory_.NewLiteral(token_.GetValue(),
                                                    Literal::kString));

  Advance();

  if (token_ != TokenKind::KW_AS) {
    ErrorMsg(boost::format("expected as keyword"));
    return ParserResult<Statement>(); // Error
  }

  Advance();
  ValidToken();

  if (token_ != TokenKind::IDENTIFIER) {
    ErrorMsg(boost::format("expected identifier"));
    return ParserResult<Statement>(); // Error
  }

  std::unique_ptr<Identifier> id(factory_.NewIdentifier(
      boost::get<std::string>(token_.GetValue())));

  Advance();

  return ParserResult<Statement>(
        factory_.NewImportStatement(std::move(path), std::move(id)));
}

ParserResult<Statement> Parser::ParserStmtDecl() {
  if (token_ == TokenKind::KW_FUNC) {
    ParserResult<FunctionDeclaration> func(ParserFunctionDeclaration(false));
    return ParserResult<Statement>(func.MoveAstNode<Statement>());
  } else if (token_ == TokenKind::KW_CMD) {
    ParserResult<Declaration> cmd(ParserCmdDeclaration());
    return ParserResult<Statement>(cmd.MoveAstNode<Statement>());
  } else if (token_ == TokenKind::KW_CLASS) {
    ParserResult<Declaration> class_decl(ParserClassDecl());
    return ParserResult<Statement>(class_decl.MoveAstNode<Statement>());
  } else if (token_ == TokenKind::KW_ALIAS) {
    ParserResult<Declaration> alias(ParserAliasDeclaration());
    return ParserResult<Statement>(alias.MoveAstNode<Statement>());
  }

  return ParserResult<Statement>(); // error
}

bool Parser::IsStmtDecl() {
  return token_.IsAny(TokenKind::KW_FUNC, TokenKind::KW_CMD,
                      TokenKind::KW_CLASS, TokenKind::KW_ALIAS);
}

ParserResult<Declaration> Parser::ParserCmdDeclaration() {
  if (token_ != TokenKind::KW_CMD) {
    ErrorMsg(boost::format("expected cmd"));
    return ParserResult<Declaration>(); // Error
  }

  Advance();

  if (token_ != TokenKind::IDENTIFIER) {
    ErrorMsg(boost::format("expected identifier"));
    return ParserResult<Declaration>(); // Error
  }

  std::unique_ptr<Identifier> id(factory_.NewIdentifier(
      boost::get<std::string>(token_.GetValue())));

  Advance();

  std::unique_ptr<Block> block(ParserBlock().MoveAstNode<Block>());

  return ParserResult<Declaration>(
      factory_.NewCmdDeclaration(std::move(id), std::move(block)));
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
    std::unique_ptr<AssignableValue> value_;

    if (token_ == TokenKind::ELLIPSIS) {
      variadic = true;
      Advance();
      ValidToken();
    } else if (token_ == TokenKind::ASSIGN){
      Advance();
      ValidToken();
      value_ = ParserAssignable().MoveAstNode();
    }

    std::unique_ptr<FunctionParam> param(factory_.NewFunctionParam(
        std::move(id), std::move(value_), variadic));
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
      std::move(vec_params), true);
}

ParserResult<Statement> Parser::ParserDeferStmt() {
  Advance();
  ValidToken();

  if (token_ == TokenKind::KW_DEFER) {
    ErrorMsg(boost::format("defer not allowed inside defer"));
    return ParserResult<Statement>(); // Error
  }

  std::unique_ptr<Statement> stmt(ParserDeferableStmt());

  return ParserResult<Statement>(factory_.NewDeferStatement(std::move(stmt)));
}

std::unique_ptr<Statement> Parser::ParserDeferableStmt() {
  std::unique_ptr<Statement> stmt;

  if (token_ == TokenKind::KW_IF) {
    stmt = ParserIfStmt().MoveAstNode();
  } else if (token_ == TokenKind::KW_WHILE) {
    stmt = ParserWhileStmt().MoveAstNode();
  } else if (token_ == TokenKind::KW_CONTINUE) {
    stmt = ParserContinueStmt().MoveAstNode();
  } else if (token_ == TokenKind::KW_SWITCH) {
    stmt = ParserSwitchStmt().MoveAstNode();
  } else if (token_ == TokenKind::KW_DEL) {
    stmt = ParserDelStmt().MoveAstNode();
  } else if (token_ == TokenKind::KW_FOR) {
    stmt = ParserForInStmt().MoveAstNode();
  } else if (token_ == TokenKind::LBRACE) {
    stmt = ParserBlock().MoveAstNode();
  } else if (MatchLangStmt()) {
    stmt = ParserSimpleStmt().MoveAstNode();
  } else {
    stmt = ParserCmdFull().MoveAstNode();
  }

  return stmt;
}

ParserResult<FunctionDeclaration> Parser::ParserFunctionDeclaration(
    bool lambda) {
  if (token_ != TokenKind::KW_FUNC) {
    ErrorMsg(boost::format("expected function"));
    return ParserResult<FunctionDeclaration>(); // Error
  }

  Position pos = {token_.Line(), token_.Col()};

  Advance();
  ValidToken();

  std::unique_ptr<Identifier> id;

  // If is a lambda function, the function doesn't have identifier name
  if (!lambda) {
    if (token_ != TokenKind::IDENTIFIER) {
      ErrorMsg(boost::format("expected identifier"));
      return ParserResult<FunctionDeclaration>(); // Error
    }

    id = std::move(factory_.NewIdentifier(boost::get<std::string>(
        token_.GetValue()), std::move(nullptr)));

    Advance();
    ValidToken();
  }

  if (token_ != TokenKind::LPAREN) {
    ErrorMsg(boost::format("expected token '(' got %1%")% TokenValueStr());
    return ParserResult<FunctionDeclaration>(); // Error
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
      return ParserResult<FunctionDeclaration>(); // Error
    }

    if (!ok) {
      return ParserResult<FunctionDeclaration>(); // Error
    }

    Advance();
    ValidToken();
  }

  std::unique_ptr<Block> block(ParserBlock().MoveAstNode<Block>());

  return ParserResult<FunctionDeclaration>(factory_.NewFunctionDeclaration(
      std::move(func_params), std::move(id), std::move(block), pos));
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
  ParserResult<Expression> exp;
  std::vector<std::unique_ptr<CaseStatement>> case_list;
  std::unique_ptr<DefaultStatement> default_stmt;

  Advance();

  if (ValidToken() == TokenKind::LBRACE) {
    exp = std::move(nullptr);
  } else {
    exp = std::move(ParserOrExp());
  }

  if (token_ != TokenKind::LBRACE) {
    ErrorMsg(boost::format("expected { token"));
    return ParserResult<Statement>(); // Error
  }

  Advance();

  // parser switch block
  while (ValidToken().IsAny(TokenKind::KW_CASE, TokenKind::KW_DEFAULT)) {
    if (token_ == TokenKind::KW_CASE) {
      case_list.push_back(std::move(ParserCaseStmt()));
    } else if (token_ == TokenKind::KW_DEFAULT) {
      default_stmt = std::move(ParserDefaultStmt());
    } else {
      ErrorMsg(boost::format("expected case or default"));
      return ParserResult<Statement>(); // Error
    }
  }

  if (token_ != TokenKind::RBRACE) {
    ErrorMsg(boost::format("expected } token"));
    return ParserResult<Statement>(); // Error
  }

  Advance();

  return ParserResult<Statement>(factory_.NewSwitchStatement(exp.MoveAstNode(),
      std::move(case_list), std::move(default_stmt)));
}

std::unique_ptr<CaseStatement> Parser::ParserCaseStmt() {
  Advance();
  ValidToken();

  ParserResult<ExpressionList> exp_list(ParserExpList());
  ValidToken();

  if (token_ != TokenKind::LBRACE) {
    ErrorMsg(boost::format("expected { token"));
  }

  std::unique_ptr<Block> block(ParserBlock().MoveAstNode<Block>());

  return std::unique_ptr<CaseStatement>(factory_.NewCaseStatement(
      exp_list.MoveAstNode(), std::move(block)));
}

ParserResult<Statement> Parser::ParserBlock() {
  if (token_ != TokenKind::LBRACE) {
    ErrorMsg(boost::format("expected { token, got %1%")% TokenValueStr());
      return ParserResult<Statement>(); // Error
  }

  Advance();
  ValidToken();

  ParserResult<StatementList> stmt_list(ParserStmtList());

  // handle empty block
  if (ValidToken() == TokenKind::RBRACE) {
    Advance();
    return ParserResult<Statement>(factory_.NewBlock(stmt_list.MoveAstNode()));
  }

  if (!stmt_list) {
    return ParserResult<Statement>();
  }

  if (ValidToken() != TokenKind::RBRACE) {
    ErrorMsg(boost::format("expected } token, got %1%")% TokenValueStr());
      return ParserResult<Statement>(); // Error
  }

  Advance();

  return ParserResult<Statement>(factory_.NewBlock(stmt_list.MoveAstNode()));
}

ParserResult<StatementList> Parser::ParserStmtList() {
  std::vector<std::unique_ptr<Statement>> stmt_list;

  // Check RBRACE because it is the end of block, and as the
  // stmt list is inside the block, it has to check token
  // RBRACE(}) to know if it is in the end of the block
  while (token_.IsNot(TokenKind::EOS, TokenKind::RBRACE)) {
    // ignore new line or ; if it come before any statement
    if ((token_ == TokenKind::NWL) || (token_ == TokenKind::SEMI_COLON)) {
      Advance();
      continue;
    }

    ValidToken();
    ParserResult<Statement> stmt(ParserStmt());
    if (!stmt) {
      return ParserResult<StatementList>();
    }

    stmt_list.push_back(stmt.MoveAstNode());

    if (token_.IsAny(TokenKind::EOS, TokenKind::RBRACE)) {
      // end of file and end of block are a valid end for statement
      break;
    } else {
      // any other char than end, continue, and the statement will decide
      // if the char in valid or not
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
  ParserResult<Statement> res;
  bool check_end_stmt = false;

  if (token_ == TokenKind::KW_IF) {
    res = std::move(ParserIfStmt());
  } else if (token_ == TokenKind::KW_WHILE) {
    res = std::move(ParserWhileStmt());
  } else if (token_ == TokenKind::KW_BREAK) {
    check_end_stmt = true;
    res = std::move(ParserBreakStmt());
  } else if (token_ == TokenKind::KW_CONTINUE) {
    check_end_stmt = true;
    res = std::move(ParserContinueStmt());
  } else if (token_ == TokenKind::KW_RETURN) {
    check_end_stmt = true;
    res = std::move(ParserReturnStmt());
  } else if (token_ == TokenKind::KW_SWITCH) {
    res = std::move(ParserSwitchStmt());
  } else if (token_ == TokenKind::KW_FOR) {
    res = std::move(ParserForInStmt());
  } else if (token_ == TokenKind::KW_DEFER) {
    check_end_stmt = true;
    res = std::move(ParserDeferStmt());
  } else if (token_ == TokenKind::KW_DEL) {
    check_end_stmt = true;
    res = std::move(ParserDelStmt());
  } else if (token_ == TokenKind::KW_IMPORT) {
    check_end_stmt = true;
    res = std::move(ParserImportStmt());
  } else if (token_ == TokenKind::LBRACE) {
    res = std::move(ParserBlock());
  } else if (IsStmtDecl()) {
    res = std::move(ParserStmtDecl());
  } else if (MatchLangStmt()) {
    check_end_stmt = true;
    res = std::move(ParserSimpleStmt());
  } else {
    check_end_stmt = true;
    res = std::move(ParserCmdFull());
  }

  if (check_end_stmt) {
    if (token_.IsAny(TokenKind::EOS, TokenKind::RBRACE)) {
      return res;
    } else if ((token_ == TokenKind::NWL) ||
               (token_ == TokenKind::SEMI_COLON)) {
      Advance();
    } else {
      ErrorMsg(boost::format("expected end of statement, got %1%")%
               TokenValueStr());
      return ParserResult<Statement>(); // Error
    }
  }

  return res;
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
  auto check_pipe = [&]()-> bool {
    if (token_.Is(TokenKind::BIT_OR)) {
      Advance();
      ValidToken();
      return true;
    } else {
      return false;
    }
  };

  std::vector<std::unique_ptr<Cmd>> cmds;

  do {
    ParserResult<Statement> cmd = ParserIoRedirectCmdList();
    cmds.push_back(cmd.MoveAstNode<Cmd>());
  } while (check_pipe());

  // return a simple command if it has not a pipe command
  if (cmds.size() == 1) {
    return ParserResult<Statement>(std::move(cmds.at(0)));
  }

  return ParserResult<Statement>(factory_.NewCmdPipeSequence(std::move(cmds)));
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
      ParserResult<Cmd> exp(ParserExpCmd());
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
      ParserResult<Cmd> exp(ParserExpCmd());
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

ParserResult<Declaration> Parser::ParserAliasDeclaration() {
  Advance();  // advance alias keyword

  if (token_ != TokenKind::IDENTIFIER) {
    ErrorMsg(boost::format("expected identifier"));
    return ParserResult<Declaration>(); // Error
  }

  std::unique_ptr<Identifier> id(factory_.NewIdentifier(
      boost::get<std::string>(token_.GetValue())));

  Advance();

  if (token_ != TokenKind::ASSIGN) {
    ErrorMsg(boost::format("expected assign operator"));
    return ParserResult<Declaration>(); // Error
  }

  Advance();
  ValidToken();

  std::unique_ptr<SimpleCmd> cmd(ParserSimpleCmd().MoveAstNode<SimpleCmd>());

  return ParserResult<Declaration>(factory_.NewAliasDeclaration(
      std::move(cmd), std::move(id)));
}

ParserResult<Cmd> Parser::ParserExpCmd() {
  // Parser an expression inside command
  // ex: any_cmd -e ${v[0] + 1} -d
  if (token_ != TokenKind::DOLLAR_LBRACE) {
    ErrorMsg(boost::format("expected ${ token"));
    return ParserResult<Cmd>(); // Error
  }

  Advance();
  if (token_ == TokenKind::NWL) {
    ErrorMsg(boost::format("New line not allowed"));
    return ParserResult<Cmd>(); // Error
  }

  ParserResult<Expression> exp(ParserOrExp());

  if (token_ != TokenKind::RBRACE) {
    ErrorMsg(boost::format("token '}' expected"));
    return ParserResult<Cmd>(); // Error
  }

  bool has_space = token_.BlankAfter();

  Advance();

  return ParserResult<Cmd>(factory_.NewCmdValueExpr(exp.MoveAstNode(),
                                                    has_space));
}

ParserResult<Statement> Parser::ParserReturnStmt() {
  if (token_ != TokenKind::KW_RETURN) {
    ErrorMsg(boost::format("expected return token, got %1%")% TokenValueStr());
    return ParserResult<Statement>(); // Error
  }

  Advance();

  // Parser return for void functions
  if (IsEndOfStmt()) {
    return ParserResult<Statement>(factory_.NewReturnStatement(
        std::unique_ptr<AssignableList>(nullptr)));
  }

  ParserResult<AssignableList> list(ParserAssignableList());

  return ParserResult<Statement>(factory_.NewReturnStatement(
      std::move(list.MoveAstNode())));
}

ParserResult<Statement> Parser::ParserBreakStmt() {
  if (token_ != TokenKind::KW_BREAK) {
    ErrorMsg(boost::format("expected break token, got %1%")% TokenValueStr());
    return ParserResult<Statement>(); // Error
  }

  Advance();

  return ParserResult<Statement>(factory_.NewBreakStatement());
}

ParserResult<Statement> Parser::ParserContinueStmt() {
  // advance continue token
  Advance();

  return ParserResult<Statement>(factory_.NewContinueStatement());
}

std::unique_ptr<DefaultStatement> Parser::ParserDefaultStmt() {
  Advance();

  std::unique_ptr<Block> block(ParserBlock().MoveAstNode<Block>());

  return std::unique_ptr<DefaultStatement>(
        factory_.NewDefaultStatement(std::move(block)));
}

ParserResult<Statement> Parser::ParserSimpleStmt() {
  enum Type {kErro, kAssign, kExpStm};
  Type type = kErro;
  std::vector<std::unique_ptr<Expression>> vec_list;
  ParserResult<AssignableList> rvalue_list;
  size_t num_comma = 0;
  TokenKind kind;

  // parser the left side, if there is a comma, or assign token
  // the stmt must has a right value list
  do {
    ValidToken();
    ParserResult<Expression> exp = ParserPostExp();
    vec_list.push_back(exp.MoveAstNode());

    if (token_.Is(TokenKind::COMMA)) {num_comma++;}
  } while (CheckComma());

  if ((num_comma > 0) && token_.IsNot(TokenKind::ASSIGN)) {
    ErrorMsg(boost::format("assign expected"));
    return ParserResult<Statement>(); // Error
  }

  type = kExpStm;

  // After find a assign token, parser an assignable list
  if (Token::IsAssignToken(token_.GetKind())) {
    type = Type::kAssign;
    kind = token_.GetKind();

    Advance(); // consume assign token
    ValidToken();

    rvalue_list = ParserAssignableList();
  }

  switch (type) {
    // return an assignment statement
    case Type::kAssign:
      return ParserResult<Statement>(factory_.NewAssignmentStatement(
          kind, factory_.NewExpressionList(std::move(vec_list)),
          rvalue_list.MoveAstNode()));
      break;

    // return an expression statement
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

ParserResult<Statement> Parser::ParserDelStmt() {
  // advance del token
  Advance();
  ValidToken();

  std::unique_ptr<ExpressionList> exp_ls(ParserPostExpList().MoveAstNode());

  return ParserResult<Statement>(factory_.NewDelStatement(std::move(exp_ls)));
}

ParserResult<ExpressionList> Parser::ParserPostExpList() {
  std::vector<std::unique_ptr<Expression>> vec_list;

  do {
    ValidToken();
    ParserResult<Expression> exp(ParserPostExp());
    vec_list.push_back(exp.MoveAstNode());
  } while (CheckComma());

  return ParserResult<ExpressionList>(factory_.NewExpressionList(
      std::move(vec_list)));
}

ParserResult<AssignableValue> Parser::ParserAssignable() {
  if (token_ == TokenKind::KW_FUNC) {
    ParserResult<FunctionDeclaration> flambda(ParserFunctionDeclaration(true));
    return ParserResult<AssignableValue>(factory_
        .NewAssignableValue<FunctionDeclaration>(flambda.MoveAstNode()));
  } else {
    ParserResult<Expression> exp(ParserOrExp());
    return ParserResult<AssignableValue>(
        factory_.NewAssignableValue<Expression>(exp.MoveAstNode()));
  }
}

ParserResult<AssignableList> Parser::ParserAssignableList() {
  std::vector<std::unique_ptr<AssignableValue>> vec_values;

  do {
    ValidToken();
    ParserResult<AssignableValue> value(ParserAssignable());
    vec_values.push_back(value.MoveAstNode());
  } while (CheckComma());

  return ParserResult<AssignableList>(factory_.NewAssignableList(
      std::move(vec_values)));
}

ParserResult<ExpressionList> Parser::ParserExpList() {
  std::vector<std::unique_ptr<Expression>> vec_exp;

  do {
    ValidToken();
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
  if (token_.Is(TokenKind::KW_NOT)) {
    TokenKind token_kind = token_.GetKind();
    Advance(); // Consume the token
    ValidToken();

    ParserResult<Expression> exp(ParserNotExp());
    return ParserResult<Expression>(factory_.NewNotExpression(
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

  while (token_.IsAny(TokenKind::MUL, TokenKind::DIV, TokenKind::MOD)) {
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
  if (IsUnaryOp()) {
    TokenKind token_kind = token_.GetKind();
    Advance(); // Consume the token
    ValidToken();

    ParserResult<Expression> exp = ParserUnaryExp();
    return ParserResult<Expression>(factory_.NewUnaryOperation(
          token_kind, exp.MoveAstNode()));
  }

  return ParserPostExp();
}

ParserResult<Expression> Parser::ParserPostExp() {
  ParserResult<Expression> exp = ParserPrimaryExp();

  while (token_.IsAny(TokenKind::LBRACKET, TokenKind::DOT,
         TokenKind::LPAREN)) {
    if (token_ == TokenKind::LBRACKET) {
      // parser array
      Advance();
      ValidToken();
      ParserResult<Expression> index_exp(ParserSlice());

      if (ValidToken().IsNot(TokenKind::RBRACKET)) {
        ErrorMsg(boost::format("Expected ']' in the end of expression"));
        return ParserResult<Expression>(); // Error
      }
      Advance();

      exp = factory_.NewArray(exp.MoveAstNode(),index_exp.MoveAstNode());
    } else if (token_ == TokenKind::DOT) {
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

      std::vector<std::unique_ptr<AssignableValue>> rlist;

      if (ValidToken().Is(TokenKind::RPAREN)) {
        // empty expression list
        exp = factory_.NewFunctionCall(
            exp.MoveAstNode(), factory_.NewAssignableList(
                std::move(rlist)));
      } else {
        // Parser expression list separted by (,) comma
        auto rvalue_list = ParserAssignableList();
        exp = factory_.NewFunctionCall(exp.MoveAstNode(),
                                       rvalue_list.MoveAstNode());

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

std::tuple<std::unique_ptr<KeyValue>, bool> Parser::ParserKeyValue() {
  ParserResult<Expression> exp(ParserOrExp());

  ValidToken();

  if (token_ != TokenKind::COLON) {
    ErrorMsg(boost::format("Expected token ':', got %1%")
        % Token::TokenValueToStr(token_.GetValue()));

    return std::tuple<std::unique_ptr<KeyValue>, bool>(
        std::unique_ptr<KeyValue>(nullptr), false);
  }

  Advance();
  ValidToken();

  ParserResult<AssignableValue> rvalue(ParserAssignable());

  std::unique_ptr<KeyValue> key_value(
      factory_.NewKeyValue(exp.MoveAstNode(), rvalue.MoveAstNode()));

  return std::tuple<std::unique_ptr<KeyValue>, bool>(
      std::move(key_value), true);
}

ParserResult<Expression> Parser::ParserDictionary() {
  // Advance token {
  Advance();
  ValidToken();

  std::vector<std::unique_ptr<KeyValue>> key_value_list;

  if (token_ == TokenKind::RBRACE) {
    ParserResult<Expression> dic(factory_.NewDictionaryInstantiation(
        std::move(key_value_list)));
    Advance();
    return dic;
  }

  do {
    ValidToken();
    std::unique_ptr<KeyValue> key_value;
    bool ok;
    std::tie(key_value, ok) = ParserKeyValue();
    key_value_list.push_back(std::move(key_value));
  } while (CheckComma());

  ValidToken();

  if (token_ != TokenKind::RBRACE) {
    ErrorMsg(boost::format("Expected token '}', got %1%")
        % Token::TokenValueToStr(token_.GetValue()));

    return ParserResult<Expression>();
  }

  Advance();

  return ParserResult<Expression>(factory_.NewDictionaryInstantiation(
      std::move(key_value_list)));
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

ParserResult<Expression> Parser::ParserArrayInstantiation() {
  // Advance token '[' and goes until a token different from new line
  Advance();
  ValidToken();

  // Parser empty array
  if (token_ == TokenKind::RBRACKET) {
    ParserResult<Expression> arr(factory_.NewArrayInstantiation(
      std::unique_ptr<AssignableList>(nullptr)));

    Advance();
    return arr;
  }

  auto rvalue_list = ParserAssignableList();

  ValidToken();
  if (token_ != TokenKind::RBRACKET) {
    ErrorMsg(boost::format("Expected token ]"));
    return ParserResult<Expression>(); // Error
  }

  Advance();

  ParserResult<Expression> arr(factory_.NewArrayInstantiation(
      rvalue_list.MoveAstNode()));

  return arr;
}

ParserResult<Expression> Parser::ParserSlice() {
  bool is_slice = false;
  bool has_start = false;
  bool has_end = false;

  ParserResult<Expression> start_exp;
  ParserResult<Expression> end_exp;

  ValidToken();

  // Verify if slice has the first part => start:end
  if (token_ == TokenKind::COLON) {
    goto END_PART;
  }

  start_exp = ParserOrExp();
  has_start = true;

  ValidToken();

  END_PART:
  if (token_ == TokenKind::COLON) {
    is_slice = true;
    Advance();
    ValidToken();

    // Verify if slice has the second part => start:end
    if (token_.IsAny(TokenKind::RBRACKET, TokenKind::COMMA)) {
      goto FINISH;
    }

    // Parser expression only if slice
    end_exp = ParserOrExp();
    has_end = true;
  }

  FINISH:
  if (is_slice) {
    return ParserResult<Expression>(factory_.NewSlice(
        has_start? start_exp.MoveAstNode() : nullptr,
        has_end? end_exp.MoveAstNode() : nullptr));
  } else {
    // If it is not slice, only the start expression is valid
    // because only slice has token :
    return start_exp;
  }
}

ParserResult<Expression> Parser::ParserPrimaryExp() {
  if (token_ == TokenKind::IDENTIFIER) {
    // parser scope id: scope1::scope2::id
    return ParserScopeIdentifier();
  } else if (token_ == TokenKind::DOLLAR_LPAREN) {
    // parser expression command: $(ls)
    Advance(); // consume the token '$('

    std::unique_ptr<CmdFull> cmd(ParserCmdFull().MoveAstNode<CmdFull>());
    Advance(); // consume the token ')'

    ParserResult<Expression> res(factory_.NewCmdExpression(std::move(cmd)));
    return res;
  } else if (token_ == TokenKind::LBRACKET) {
    // parser array instantiation: [a, b, c]
    return ParserArrayInstantiation();
  } else if (token_ == TokenKind::LBRACE) {
    // parser dictionary instantiation: {a, b, c}
    return ParserDictionary();
  } else if (token_ == TokenKind::LPAREN) {
    // parser expression: (4+3)
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
  } else if (token.Is(TokenKind::KW_TRUE)) {
    Token::Value value = true;
    return ParserResult<Expression>(factory_.NewLiteral(value, Literal::kBool));
  } else if (token.Is(TokenKind::KW_FALSE)) {
    Token::Value value = false;
    return ParserResult<Expression>(factory_.NewLiteral(value, Literal::kBool));
  } else if (token.Is(TokenKind::KW_NULL)) {
    return ParserResult<Expression>(factory_.NewNullExpression());
  } else {
    ErrorMsg(boost::format("primary expression expected, got %1%")
        % Token::TokenValueToStr(token.GetValue()));
    SetTokenError(token);
    return ParserResult<Expression>(); // Error
  }
}

}
}
