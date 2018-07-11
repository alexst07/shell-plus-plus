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

namespace shpp {
namespace internal {

ParserResult<Statement> Parser::ParserImportStmt() {
  Advance();
  ValidToken();

  if (token_ == TokenKind::IDENTIFIER) {
    return ParserImportIdListStmt();
  } else if (token_ == TokenKind::STRING_LITERAL) {
    return ParserImportPathStmt();
  } else if (token_ == TokenKind::MUL) {
    return ParserImportStarStmt();
  } else {
    ErrorMsg(boost::format("import statement not valid"));
    return ParserResult<Statement>(); // Error
  }
}

ParserResult<Statement> Parser::ParserImportIdListStmt() {
  std::vector<std::unique_ptr<Identifier>> id_list;

  id_list = ParserIdList();

  if (id_list.empty()) {
    return ParserResult<Statement>(); // Error
  }

  if (token_ != TokenKind::KW_FROM) {
    ErrorMsg(boost::format("expected from keyword, got %1%")% TokenValueStr());
    return ParserResult<Statement>(); // Error
  }

  // advance from keyword
  Advance();

  if (token_ != TokenKind::STRING_LITERAL) {
    ErrorMsg(boost::format("expected string path"));
    return ParserResult<Statement>(); // Error
  }

  std::string from = boost::get<std::string>(token_.GetValue());

  Advance();

  return ParserResult<Statement>(
      factory_.NewImportStatement(from, std::move(id_list), /*star*/false));
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

ParserResult<Statement> Parser::ParserImportStarStmt() {
  // advance MUL token
  Advance();

  if (token_ != TokenKind::KW_FROM) {
    ErrorMsg(boost::format("expected from keyword, got %1%")% TokenValueStr());
    return ParserResult<Statement>(); // Error
  }

  // advance from keyword
  Advance();

  if (token_ != TokenKind::STRING_LITERAL) {
    ErrorMsg(boost::format("expected string path"));
    return ParserResult<Statement>(); // Error
  }

  std::string from = boost::get<std::string>(token_.GetValue());

  Advance();

  return ParserResult<Statement>(factory_.NewImportStatement(from,
      std::unique_ptr<Literal>(nullptr), /*star*/ true));
}

ParserResult<Statement> Parser::ParserStmtDecl() {
  if (token_ == TokenKind::KW_FUNC) {
    ParserResult<AstNode> func(ParserFunctionDeclaration(false));
    return ParserResult<Statement>(func.MoveAstNode<Statement>());
  } else if (token_ == TokenKind::KW_CMD) {
    ParserResult<Declaration> cmd(ParserCmdDeclaration());
    return ParserResult<Statement>(cmd.MoveAstNode<Statement>());
  } else if (token_ == TokenKind::KW_CLASS) {
    ParserResult<Declaration> class_decl(ParserClassDecl(false, false));
    return ParserResult<Statement>(class_decl.MoveAstNode<Statement>());
  } else if (token_ == TokenKind::KW_FINAL) {
    Advance();
    ParserResult<Declaration> class_decl(ParserClassDecl(true, false));
    return ParserResult<Statement>(class_decl.MoveAstNode<Statement>());
  } else if (token_ == TokenKind::KW_ABSTRACT) {
    Advance();
    ParserResult<Declaration> class_decl(ParserClassDecl(false, true));
    return ParserResult<Statement>(class_decl.MoveAstNode<Statement>());
  } else if (token_ == TokenKind::KW_INTERFACE) {
    ParserResult<Declaration> iface_decl(ParserInterfaceDecl());
    return ParserResult<Statement>(iface_decl.MoveAstNode<Statement>());
  } else if (token_ == TokenKind::KW_ALIAS) {
    ParserResult<Declaration> alias(ParserAliasDeclaration());
    return ParserResult<Statement>(alias.MoveAstNode<Statement>());
  }

  return ParserResult<Statement>(); // error
}

bool Parser::IsStmtDecl() {
  return token_.IsAny(TokenKind::KW_FUNC, TokenKind::KW_CMD,
      TokenKind::KW_CLASS, TokenKind::KW_ALIAS, TokenKind::KW_ABSTRACT,
      TokenKind::KW_INTERFACE, TokenKind::KW_FINAL);
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
  } else if (token_ == TokenKind::KW_TRY) {
    stmt = ParserTryCatch().MoveAstNode();
  }  else if (token_ == TokenKind::LBRACE) {
    stmt = ParserBlock().MoveAstNode();
  } else if (token_ == TokenKind::KW_VARENV) {
    stmt = ParserVarEnvStmt().MoveAstNode();
  } else if (MatchLangStmt()) {
    stmt = ParserSimpleStmt().MoveAstNode();
  } else {
    stmt = ParserCmdFull().MoveAstNode();
  }

  return stmt;
}

ParserResult<AstNode> Parser::ParserFunctionDeclaration(
    bool lambda, bool abstract, bool fstatic) {
  if (token_ != TokenKind::KW_FUNC) {
    ErrorMsg(boost::format("expected function"));
    return ParserResult<AstNode>(); // Error
  }

  Position pos = {token_.Line(), token_.Col()};

  Advance();
  ValidToken();

  std::unique_ptr<Identifier> id;

  // If is a lambda function, the function doesn't have identifier name
  if (!lambda) {
    if (token_ != TokenKind::IDENTIFIER) {
      ErrorMsg(boost::format("expected identifier"));
      return ParserResult<AstNode>(); // Error
    }

    id = factory_.NewIdentifier(boost::get<std::string>(token_.GetValue()),
        std::move(nullptr));

    Advance();
    ValidToken();
  }

  if (token_ != TokenKind::LPAREN) {
    ErrorMsg(boost::format("expected token '(' got %1%")% TokenValueStr());
    return ParserResult<AstNode>(); // Error
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
      return ParserResult<AstNode>(); // Error
    }

    if (!ok) {
      return ParserResult<AstNode>(); // Error
    }

    Advance();
    ValidToken();
  }

  std::unique_ptr<Block> block;
  if (!abstract) {
    block = ParserBlock().MoveAstNode<Block>();
  }

  if (id) {
    return ParserResult<AstNode>(factory_.NewFunctionDeclaration(
      std::move(func_params), std::move(id), std::move(block), fstatic, pos));
  }

  return ParserResult<AstNode>(factory_.NewFunctionExpression(
      std::move(func_params), std::move(block), pos));
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

  ParserResult<Expression> exp(ParserLetExp());

  ValidToken();

  ParserResult<Statement> then_block(ParserBlock());

  if (!then_block) {
    return ParserResult<Statement>();
  }

  if (token_ == TokenKind::KW_ELSE) {
    ParserResult<Statement> else_block;

    Advance();

    if (ValidToken() == TokenKind::KW_IF) {
      else_block = ParserIfStmt();
    } else {
      else_block = ParserBlock();
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
    exp = ParserLetExp();
  }

  if (token_ != TokenKind::LBRACE) {
    ErrorMsg(boost::format("expected { token"));
    return ParserResult<Statement>(); // Error
  }

  Advance();

  // parser switch block
  while (ValidToken().IsAny(TokenKind::KW_CASE, TokenKind::KW_DEFAULT)) {
    if (token_ == TokenKind::KW_CASE) {
      case_list.push_back(ParserCaseStmt());
    } else if (token_ == TokenKind::KW_DEFAULT) {
      default_stmt = ParserDefaultStmt();
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

ParserResult<Statement> Parser::ParserVarEnvStmt() {
  // advance varenv key word
  Advance();
  ValidToken();

  if (token_ != TokenKind::IDENTIFIER) {
    ErrorMsg(boost::format("expected identifier"));
    return ParserResult<Statement>(); // Error
  }

  std::unique_ptr<Identifier> id(factory_.NewIdentifier(
      boost::get<std::string>(token_.GetValue())));

  Advance();
  if (ValidToken() != TokenKind::ASSIGN) {
    ErrorMsg(boost::format("expected '=' token, got %1%")% TokenValueStr());
      return ParserResult<Statement>(); // Error
  }

  Advance();
  ValidToken();

  ParserResult<Expression> exp(ParserLetExp());

  return ParserResult<Statement>(factory_.NewVarEnvStatement(
      std::move(id), exp.MoveAstNode()));
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

  ParserResult<Expression> exp(ParserLetExp());

  ValidToken();

  ParserResult<Statement> block(ParserBlock());

  return ParserResult<Statement>(factory_.NewWhileStatement(
      exp.MoveAstNode(), block.MoveAstNode()));
}

ParserResult<Statement> Parser::ParserStmt() {
  ParserResult<Statement> res;

  // if the statement has block, check_end_stmt must be false
  // but if it hasn't block, so we have to check any token
  // that is allowed after a statement
  bool check_end_stmt = false;

  if (token_ == TokenKind::KW_IF) {
    res = ParserIfStmt();
  } else if (token_ == TokenKind::KW_WHILE) {
    res = ParserWhileStmt();
  } else if (token_ == TokenKind::KW_BREAK) {
    check_end_stmt = true;
    res = ParserBreakStmt();
  } else if (token_ == TokenKind::KW_CONTINUE) {
    check_end_stmt = true;
    res = ParserContinueStmt();
  } else if (token_ == TokenKind::KW_RETURN) {
    check_end_stmt = true;
    res = ParserReturnStmt();
  } else if (token_ == TokenKind::KW_SWITCH) {
    res = ParserSwitchStmt();
  } else if (token_ == TokenKind::KW_FOR) {
    res = ParserForInStmt();
  } else if (token_ == TokenKind::KW_DEFER) {
    check_end_stmt = true;
    res = ParserDeferStmt();
  } else if (token_ == TokenKind::KW_GLOBAL) {
    check_end_stmt = true;
    res = ParserGlobalAssignment();
  } else if (token_ == TokenKind::KW_DEL) {
    check_end_stmt = true;
    res = ParserDelStmt();
  } else if (token_ == TokenKind::KW_IMPORT) {
    check_end_stmt = true;
    res = ParserImportStmt();
  } else if (token_ == TokenKind::KW_TRY) {
    res = ParserTryCatch();
  } else if (token_ == TokenKind::KW_VARENV) {
    res = ParserVarEnvStmt();
  } else if (token_ == TokenKind::KW_THROW) {
    check_end_stmt = true;
    res = ParserThrow();
  } else if (token_ == TokenKind::LBRACE) {
    res = ParserBlock();
  } else if (IsStmtDecl()) {
    res = ParserStmtDecl();
  } else if (MatchLangStmt()) {
    check_end_stmt = true;
    res = ParserSimpleStmt();
  } else if (token_ == TokenKind::KW_CATCH) {
    ErrorMsg(boost::format("catch must be after try block"));
    return ParserResult<Statement>(); // Error
  } else if (token_ == TokenKind::KW_FINALLY) {
    ErrorMsg(boost::format("finally must be after try catch block"));
    return ParserResult<Statement>(); // Error
  } else {
    check_end_stmt = true;
    res = ParserCmdFull();
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
    ErrorMsg(boost::format("unexpected token %1% in the end of command")%
        TokenValueStr());
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

    rcmd = ParserCmdPipe();

    if (rcmd) {
      lcmd = factory_.NewCmdAndOr(token_kind, lcmd.MoveAstNode<Cmd>(),
          rcmd.MoveAstNode<Cmd>());
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
  if (CmdValidInt() || IsIOToken(token_) || CmdValidAnd()) {
    return true;
  }

  return false;
}

ParserResult<Statement> Parser::ParserSubShell() {
  Advance();

  bool self_process = false;

  if (token_ == TokenKind::BIT_AND) {
    self_process = true;
    Advance();
  }

  ParserResult<Statement> block(ParserBlock());

  return ParserResult<Statement>(factory_.NewSubShell(block.MoveAstNode<Block>(),
      self_process));
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
    std::unique_ptr<CmdIoRedirect> io(ParserIoRedirectCmd());
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
    integer = factory_.NewLiteral(token_.GetValue(), Literal::kInteger);
    Advance();
  } else if (CmdValidAnd()) {
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
      pieces.push_back(exp.MoveAstNode());
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

  // parser subshell if key word sub in found
  if (token_ == TokenKind::KW_SHELL) {
    return ParserSubShell();
  }

  int num_pieces = 0;
  while (!IsCmdStopPoint()) {
    // Count if the command has some pieces
    num_pieces++;

    // Parser an expression inside command
    // ex: cmd -e ${v[0] + 1} -d
    if (IsCmdExprToken()) {
      ParserResult<Cmd> exp(ParserExpCmd());
      pieces.push_back(exp.MoveAstNode());
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
  bool is_iterator = false;

  // check if the token is $@{, on this case it is an iterator cmd expression
  if (token_ == TokenKind::DOLLAR_AT_LBRACE) {
    is_iterator = true;
  }

  Advance();

  if (token_ == TokenKind::NWL) {
    ErrorMsg(boost::format("New line not allowed"));
    return ParserResult<Cmd>(); // Error
  }

  ParserResult<Expression> exp(ParserLetExp());

  if (token_ != TokenKind::RBRACE) {
    ErrorMsg(boost::format("token '}' expected"));
    return ParserResult<Cmd>(); // Error
  }

  bool has_space = token_.BlankAfter();

  Advance();

  return ParserResult<Cmd>(factory_.NewCmdValueExpr(exp.MoveAstNode(),
                                                    has_space, is_iterator));
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
      list.MoveAstNode()));
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

ParserResult<Statement> Parser::ParserSimpleStmt(bool force_assignment) {
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

  // if there are comma on statement, so it has to be assignment statement
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

  // check if force_assignment is true, so the statment has to be assignment
  // operation, it is useful in let expression
  if (force_assignment && type != Type::kAssign) {
    ErrorMsg(boost::format("expected assignment operation"));
    return ParserResult<Statement>(); // Error
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

ParserResult<Statement> Parser::ParserGlobalAssignment() {
  // advance global token
  Advance();

  std::vector<std::unique_ptr<Expression>> vec_list;

  do {
    ValidToken();

    if (token_ != TokenKind::IDENTIFIER) {
      ErrorMsg(boost::format("expected identifier token"));
      return ParserResult<Statement>(); // Error
    }

    std::unique_ptr<Expression> id(
        factory_.NewIdentifier(boost::get<std::string>(token_.GetValue())));

    vec_list.push_back(std::move(id));
    Advance();
  } while (CheckComma());

  if (token_.IsNot(TokenKind::ASSIGN)) {
    ErrorMsg(boost::format("assign operator expected"));
    return ParserResult<Statement>(); // Error
  }

  Advance();
  ValidToken();

  ParserResult<AssignableList> rvalue_list = ParserAssignableList();

  std::unique_ptr<AssignmentStatement> assign(factory_.NewAssignmentStatement(
      TokenKind::ASSIGN, factory_.NewExpressionList(std::move(vec_list)),
      rvalue_list.MoveAstNode()));

  return ParserResult<Statement>(factory_.NewGlobalAssignmentStatement(
      std::move(assign)));
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

std::vector<std::unique_ptr<Identifier>> Parser::ParserIdList() {
  std::vector<std::unique_ptr<Identifier>> id_list;

  do {
    ValidToken();

    if (token_ != TokenKind::IDENTIFIER) {
      ErrorMsg(boost::format("Expected token ':', got %1%")
          % Token::TokenValueToStr(token_.GetValue()));

      return std::vector<std::unique_ptr<Identifier>>();
    }

    std::unique_ptr<Identifier> id(
        factory_.NewIdentifier(boost::get<std::string>(token_.GetValue())));
    id_list.push_back(std::move(id));
    Advance();
  } while (CheckComma());

  return id_list;
}

ParserResult<AssignableValue> Parser::ParserAssignable() {
  if (token_ == TokenKind::KW_LAMBDA) {
    ParserResult<Expression> flambda(ParserLambda());
    return ParserResult<AssignableValue>(
        factory_.NewAssignableValue<Expression>(flambda.MoveAstNode()));
  } else {
    ParserResult<Expression> exp(ParserLetExp());
    return ParserResult<AssignableValue>(
        factory_.NewAssignableValue<Expression>(exp.MoveAstNode()));
  }
}

ParserResult<Expression> Parser::ParserLambda() {
  // advance lambda token
  Advance();

  std::vector<std::unique_ptr<FunctionParam>> func_params;
  bool ok = true;

  if (token_ == TokenKind::COLON) {
    // lambda without parameters
    Advance();
  } else {
    std::tie(func_params, ok) = ParserParamsList();

    if (token_ != TokenKind::COLON) {
      ErrorMsg(boost::format("Expected token ':', got %1%")
          % Token::TokenValueToStr(token_.GetValue()));

      return ParserResult<Expression>();
    }

    Advance();
  }

  Position pos = {token_.Line(), token_.Col()};

  // mount assignment value
  std::vector<std::unique_ptr<AssignableValue>> vec_values;
  ParserResult<AssignableValue> value(ParserAssignable());
  vec_values.push_back(value.MoveAstNode());

  // mount assignment libboost-all-dev
  ParserResult<AssignableList> assign_list(factory_.NewAssignableList(
      std::move(vec_values)));

  // mount return statement
  ParserResult<Statement> ret_stmt(factory_.NewReturnStatement(
      assign_list.MoveAstNode()));

  // mount stmt list
  std::vector<std::unique_ptr<Statement>> stmt_vec;
  stmt_vec.push_back(ret_stmt.MoveAstNode());

  ParserResult<StatementList> stmt_list(factory_.NewStatementList(
      std::move(stmt_vec)));

  // mount the block
  ParserResult<Statement> block(factory_.NewBlock(stmt_list.MoveAstNode()));

  return ParserResult<Expression>(factory_.NewFunctionExpression(
      std::move(func_params), block.MoveAstNode<Block>(), pos));
}

ParserResult<Expression> Parser::ParserArgument() {
  if (PeekAhead().Is(TokenKind::ASSIGN)) {
    // if is named parameter extract the key name and the argument
    if (!token_.Is(TokenKind::IDENTIFIER)) {
      // expect id before token '='
      ErrorMsg(boost::format("Expected identifier on keyword argument"));
      return ParserResult<Expression>();
    }

    std::string name = boost::get<std::string>(token_.GetValue());

    // Advance id
    Advance();

    // Advance '=' token
    Advance();
    ValidToken();

    // on this case ellipsis expression isn't valid
    ParserResult<AssignableValue> value(ParserAssignable());
    return ParserResult<Expression>(factory_.NewArgument(name,
        value.MoveAstNode()));

  } else {
    // name is empty if user didnp't defined it
    if (token_.Is(TokenKind::ELLIPSIS)) {
      ParserResult<Expression> ellipsis_exp = ParserEllipsisExp();
      return ParserResult<Expression>(factory_.NewArgument("",
          ellipsis_exp.MoveAstNode<AssignableValue>()));
    } else {
      ParserResult<AssignableValue> value(ParserAssignable());
      return ParserResult<Expression>(factory_.NewArgument("",
          value.MoveAstNode()));
    }
  }
}

ParserResult<ArgumentsList> Parser::ParserArgumentsList() {
  std::vector<std::unique_ptr<Argument>> args_list;

  do {
    ValidToken();
    ParserResult<Expression> arg = ParserArgument();
    args_list.push_back(arg.MoveAstNode<Argument>());
    ValidToken();
  } while (CheckComma());

  return ParserResult<ArgumentsList>(factory_.NewArgumentsList(
      std::move(args_list)));
}

ParserResult<AssignableList> Parser::ParserAssignableList(
    std::unique_ptr<AssignableValue> first_value) {
  std::vector<std::unique_ptr<AssignableValue>> vec_values;

  // insert a initial value if it is valid
  if (first_value) {
    vec_values.push_back(std::move(first_value));

    if (token_.Is(TokenKind::COMMA)) {
      Advance();
      ValidToken();
    } else {
      return ParserResult<AssignableList>(factory_.NewAssignableList(
      std::move(vec_values)));
    }
  }

  do {
    ValidToken();
    if (token_.Is(TokenKind::ELLIPSIS)) {
      ParserResult<Expression> ellipsis_exp = ParserEllipsisExp();
      vec_values.push_back(ellipsis_exp.MoveAstNode<AssignableValue>());
    } else {
      ParserResult<AssignableValue> value(ParserAssignable());
      vec_values.push_back(value.MoveAstNode());
    }
  } while (CheckComma());

  return ParserResult<AssignableList>(factory_.NewAssignableList(
      std::move(vec_values)));
}

ParserResult<Expression> Parser::ParserEllipsisExp() {
  // advance ellipsis '...'
  Advance();

  ValidToken();
  ParserResult<Expression> expr = ParserBitOrExp();

  return ParserResult<Expression>(factory_.NewEllipsisExpression(
      expr.MoveAstNode()));
}

ParserResult<ExpressionList> Parser::ParserExpList() {
  std::vector<std::unique_ptr<Expression>> vec_exp;

  do {
    ValidToken();

    if (token_.Is(TokenKind::ELLIPSIS)) {
      ParserResult<Expression> ellipsis_exp = ParserEllipsisExp();
      vec_exp.push_back(ellipsis_exp.MoveAstNode());
    } else {
      ParserResult<Expression> exp = ParserLetExp();
      vec_exp.push_back(exp.MoveAstNode());
    }
  } while (CheckComma());

  return ParserResult<ExpressionList>(factory_.NewExpressionList(
      std::move(vec_exp)));
}

ParserResult<Expression> Parser::ParserLetExp() {
  if (token_.Is(TokenKind::KW_LET)) {
    Advance();
    ValidToken();

    // parser assignment operation
    ParserResult<Statement> assign = ParserSimpleStmt(true);
    return ParserResult<Expression>(factory_.NewLetExpression(
        assign.MoveAstNode<AssignmentStatement>()));
  }

  // if it is not let expression, so execute the if-else expression
  return ParserIfElseExp();
}

ParserResult<Expression> Parser::ParserIfElseExp() {
  ParserResult<Expression> then_exp = ParserNotExp();

  // if the keyword "if" was found on the same line, so it is
  // an if else expression
  if (token_.Is(TokenKind::KW_IF)) {
    Advance();
    ValidToken();

    ParserResult<Expression> comp_exp = ParserNotExp();

    ValidToken();
    if (!token_.Is(TokenKind::KW_ELSE)) {
      ErrorMsg(boost::format("Expected else keyword"));
      return ParserResult<Expression>();
    }

    Advance();
    ValidToken();

    ParserResult<Expression> else_exp = ParserNotExp();

    return ParserResult<Expression>(factory_.NewIfElseExpression(
        comp_exp.MoveAstNode(), then_exp.MoveAstNode(),
        else_exp.MoveAstNode()));
  }

  return then_exp;
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

  return ParserOrExp();
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

    rexp = ParserAndExp();

    if (rexp) {
      lexp = factory_.NewBinaryOperation(token_kind, lexp.MoveAstNode(),
          rexp.MoveAstNode());
    }
  }

  return lexp;
}

ParserResult<Expression> Parser::ParserAndExp() {
  ParserResult<Expression> rexp;
  ParserResult<Expression> lexp = ParserComparisonExp();

  if (!lexp) {
    return ParserResult<Expression>(); // Error
  }

  while (token_.Is(TokenKind::AND)) {
    TokenKind token_kind = token_.GetKind();
    Advance();
    ValidToken();

    rexp = ParserComparisonExp();

    if (rexp) {
      lexp = factory_.NewBinaryOperation(token_kind, lexp.MoveAstNode(),
          rexp.MoveAstNode());
    }
  }

  return lexp;
}

ParserResult<Expression> Parser::ParserComparisonExp() {
  ParserResult<Expression> rexp;
  ParserResult<Expression> lexp = ParserBitOrExp();

  if (!lexp) {
    return ParserResult<Expression>(); // Error
  }

  while (Token::IsComparisonToken(token_.GetKind()) ||
         token_.IsAny(TokenKind::KW_IN, TokenKind::KW_INSTANCEOF,
                      TokenKind::KW_IS)) {
    TokenKind token_kind = token_.GetKind();
    Advance();
    ValidToken();

    rexp = ParserBitOrExp();

    if (rexp) {
      lexp = factory_.NewBinaryOperation(token_kind, lexp.MoveAstNode(),
          rexp.MoveAstNode());
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

    rexp = ParserBitXorExp();

    if (rexp) {
      lexp = factory_.NewBinaryOperation(token_kind, lexp.MoveAstNode(),
          rexp.MoveAstNode());
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

    rexp = ParserBitAndExp();

    if (rexp) {
      lexp = factory_.NewBinaryOperation(token_kind, lexp.MoveAstNode(),
          rexp.MoveAstNode());
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

    rexp = ParserShiftExp();

    if (rexp) {
      lexp = factory_.NewBinaryOperation(token_kind, lexp.MoveAstNode(),
          rexp.MoveAstNode());
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

    rexp = ParserArithExp();

    if (rexp) {
      lexp = factory_.NewBinaryOperation(token_kind, lexp.MoveAstNode(),
          rexp.MoveAstNode());
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

    rexp = ParserTerm();

    if (rexp) {
      lexp = factory_.NewBinaryOperation(token_kind, lexp.MoveAstNode(),
          rexp.MoveAstNode());
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

    rexp = ParserUnaryExp();

    if (rexp) {
      lexp = factory_.NewBinaryOperation(token_kind, lexp.MoveAstNode(),
          rexp.MoveAstNode());
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

      if (ValidToken().Is(TokenKind::RPAREN)) {
        // empty expression list
        std::vector<std::unique_ptr<Argument>> args;
        exp = factory_.NewFunctionCall(
            exp.MoveAstNode(), factory_.NewArgumentsList(
                std::move(args)));
      } else {
        // Parser expression list separted by (,) comma
        auto args_list = ParserArgumentsList();
        exp = factory_.NewFunctionCall(exp.MoveAstNode(),
            args_list.MoveAstNode());

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
  ParserResult<Expression> exp(ParserLetExp());

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

    id = factory_.NewIdentifier(boost::get<std::string>(token_.GetValue()),
        std::move(scope));
    Advance();
  }

  ParserResult<Expression> res(std::move(id));
  return res;
}

ParserResult<ExpressionList> Parser::ParserExpNoTestList() {
  std::vector<std::unique_ptr<Expression>> vec_exp;

  do {
    ValidToken();

    if (token_.Is(TokenKind::ELLIPSIS)) {
      ParserResult<Expression> ellipsis_exp = ParserEllipsisExp();
      vec_exp.push_back(ellipsis_exp.MoveAstNode());
    } else {
      ParserResult<Expression> exp = ParserOrExp();
      vec_exp.push_back(exp.MoveAstNode());
    }
  } while (CheckComma());

  return ParserResult<ExpressionList>(factory_.NewExpressionList(
      std::move(vec_exp)));
}

ParserResult<Expression> Parser::ParserCompIf() {
  // advance if keyword
  Advance();
  ValidToken();

  ParserResult<Expression> exp(ParserOrExp());

  return ParserResult<Expression>(factory_.NewCompIf(exp.MoveAstNode()));
}

ParserResult<Expression> Parser::ParserCompFor() {
  // advance for keyword
  Advance();
  ValidToken();
  ParserResult<ExpressionList> exp_list(ParserPostExpList());

  if (token_ != TokenKind::KW_IN) {
    ErrorMsg(boost::format("expected in operator"));
    return ParserResult<Expression>(); // Error
  }

  Advance();
  ValidToken();
  ParserResult<ExpressionList> test_list(ParserExpNoTestList());

  return ParserResult<Expression>(factory_.NewCompFor(exp_list.MoveAstNode(),
      test_list.MoveAstNode()));
}

std::vector<std::unique_ptr<Expression>> Parser::ParserListComprehension() {
  std::vector<std::unique_ptr<Expression>> comp_list;

  while (token_.IsAny(TokenKind::KW_FOR, TokenKind::KW_IF)) {
    if (token_ == TokenKind::KW_FOR) {
      ParserResult<Expression> comp = ParserCompFor();
      comp_list.push_back(comp.MoveAstNode());
    } else if (token_ == TokenKind::KW_IF) {
      ParserResult<Expression> comp = ParserCompIf();
      comp_list.push_back(comp.MoveAstNode());
    }
  }

  return comp_list;
}

ParserResult<Expression> Parser::ParserTupleInstantiation() {
  // Advance token '(' and goes until a token different from new line
  Advance();
  ValidToken();

  // Parser empty array
  if (token_ == TokenKind::RPAREN) {
    ParserResult<Expression> tuple(factory_.NewTupleInstantiation(
      std::unique_ptr<AssignableList>(nullptr)));

    Advance();
    return tuple;
  }

  // get the assignable list
  std::unique_ptr<AssignableList> rvalue_list;
  rvalue_list = ParserAssignableList().MoveAstNode();

  if (ValidToken() != TokenKind::RPAREN) {
    ErrorMsg(boost::format("Expected ')' in the end of expression, got %1%")
        % TokenValueStr());
    return ParserResult<Expression>(); // Error
  }

  Advance(); // consume the token ')'

  // check if the assignable list has exactly one element, if is the case
  // only an common expression must be returned
  if (rvalue_list->size() == 1) {
    std::unique_ptr<AssignableValue> value = std::move(rvalue_list->nodes()[0]);
    ParserResult<Expression> res(std::move(value));
    return res;
  }

  return ParserResult<Expression>(factory_.NewTupleInstantiation(
      std::move(rvalue_list)));
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

  std::unique_ptr<AssignableList> rvalue_list;

  if (token_.Is(TokenKind::ELLIPSIS)) {
    // if the first element is ELLIPSIS token, it must be a simple array
    rvalue_list = ParserAssignableList().MoveAstNode();
  } else {
    // on this case we have to check, if it is a simple array or a
    // list comprehension
    std::vector<std::unique_ptr<AssignableValue>> vec_values;
    ParserResult<AssignableValue> value(ParserAssignable());

    if (token_.Is(TokenKind::KW_FOR)) {
      // it is list comprehension
      auto comp_vec = ParserListComprehension();

      ValidToken();
      if (token_ != TokenKind::RBRACKET) {
        ErrorMsg(boost::format("Expected token ]"));
        return ParserResult<Expression>(); // Error
      }

      Advance();

      // return the list comprehension
      return ParserResult<Expression>(factory_.NewListComprehension(
          value.MoveAstNode(), std::move(comp_vec)));
    } else {
      // it is an array
      rvalue_list = ParserAssignableList(value.MoveAstNode()).MoveAstNode();
    }
  }

  ValidToken();
  if (token_ != TokenKind::RBRACKET) {
    ErrorMsg(boost::format("Expected token ]"));
    return ParserResult<Expression>(); // Error
  }

  Advance();

  ParserResult<Expression> arr(factory_.NewArrayInstantiation(
      std::move(rvalue_list)));

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

  start_exp = ParserLetExp();
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
    end_exp = ParserLetExp();
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
  } else if (token_ == TokenKind::VARENVID) {
    ParserResult<Expression> varenvid(factory_.NewVarEnvId(
        boost::get<std::string>(token_.GetValue())));
    Advance();
    return varenvid;
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
    return ParserTupleInstantiation();
  } if (token_ == TokenKind::KW_FUNC) {
    ParserResult<AstNode> flambda(ParserFunctionDeclaration(true));
    return ParserResult<Expression>(flambda.MoveAstNode<Expression>());
  } else {
    return LiteralExp();
  }
}

ParserResult<Expression> Parser::ParserGlobExp(bool recursive) {
  // this method is called on LiteralExp, and it
  // was already advenced there
  std::vector<std::unique_ptr<AstNode>> pieces;

  int num_pieces = 0;
  while (token_.IsNot(TokenKind::MOD)) {
    num_pieces++;

    if (token_ == TokenKind::EOS) {
      ErrorMsg(boost::format("Invalid end before end glob"));
      return ParserResult<Expression>(); // Error
    }

    // Puts piece of the command on a vector, this vector will be the
    // entire glob expression
    auto piece = factory_.NewCmdPiece(token_);
    pieces.push_back(std::move(piece));
    Advance();
  }

  // advance the token MOD
  Advance();

  // if the glob is empty there is an error
  if (num_pieces == 0) {
    ErrorMsg(boost::format("Glob can't be empty"));
    return ParserResult<Expression>(); // Error
  }

  return ParserResult<Expression>(factory_.NewGlob(std::move(pieces),
                                                   recursive));
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
  } else if (token.Is(TokenKind::MOD)) {
    return ParserGlobExp();
  } else if (token.Is(TokenKind::RGLOB)) {
    return ParserGlobExp(true); // recursive
  } else{
    ErrorMsg(boost::format("primary expression expected, got %1%")
        % Token::TokenValueToStr(token.GetValue()));
    SetTokenError(token);
    return ParserResult<Expression>(); // Error
  }
}

}
}
