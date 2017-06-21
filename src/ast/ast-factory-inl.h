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

#ifndef SHPP_AST_H
#error This file may only be included from ast.h.
#endif

namespace shpp {
namespace internal {

class AstNodeFactory {
 public:
  AstNodeFactory(const std::function<Position()> fn_pos): fn_pos_(fn_pos) {}

  inline std::unique_ptr<Literal> NewLiteral(const Token::Value& value,
                                             Literal::Type type) {
    return std::unique_ptr<Literal>(new Literal(value, type, fn_pos_()));
  }

  inline std::unique_ptr<NullExpression> NewNullExpression() {
    return std::unique_ptr<NullExpression>(new NullExpression(fn_pos_()));
  }

  inline std::unique_ptr<Glob> NewGlob(
      std::vector<std::unique_ptr<AstNode>>&& pieces, bool recursive) {
    return std::unique_ptr<Glob>(new Glob(std::move(pieces), recursive,
                                 fn_pos_()));
  }

  inline std::unique_ptr<BinaryOperation> NewBinaryOperation(
      TokenKind token_kind, std::unique_ptr<Expression> left,
      std::unique_ptr<Expression> right) {
    return std::unique_ptr<BinaryOperation>(new BinaryOperation(
        token_kind, std::move(left), std::move(right), fn_pos_()));
  }

  inline std::unique_ptr<UnaryOperation> NewUnaryOperation(
      TokenKind token_kind, std::unique_ptr<Expression> exp) {
    return std::unique_ptr<UnaryOperation>(new UnaryOperation(
        token_kind, std::move(exp), fn_pos_()));
  }

  inline std::unique_ptr<NotExpression> NewNotExpression(
      TokenKind token_kind, std::unique_ptr<Expression> exp) {
    return std::unique_ptr<NotExpression>(new NotExpression(
        token_kind, std::move(exp), fn_pos_()));
  }

  inline std::unique_ptr<Array> NewArray(std::unique_ptr<Expression> arr_exp,
                                         std::unique_ptr<Expression> index_exp) {
    return std::unique_ptr<Array>(new Array(std::move(arr_exp),
                                            std::move(index_exp), fn_pos_()));
  }

  inline std::unique_ptr<Attribute> NewAttribute(
      std::unique_ptr<Expression> exp, std::unique_ptr<Identifier> id) {
    return std::unique_ptr<Attribute>(new Attribute(std::move(exp),
                                                    std::move(id),
                                                    fn_pos_()));
  }

  inline std::unique_ptr<Identifier> NewIdentifier(
      const std::string& name, std::unique_ptr<PackageScope> scope =
          std::unique_ptr<PackageScope>(nullptr)) {
    return std::unique_ptr<Identifier>(new Identifier(
        name, std::move(scope), fn_pos_()));
  }

  inline std::unique_ptr<PackageScope> NewPackageScope(
      std::unique_ptr<Identifier> id) {
    return std::unique_ptr<PackageScope>(new PackageScope(
        std::move(id), fn_pos_()));
  }

  inline std::unique_ptr<AssignmentStatement> NewAssignmentStatement(
      TokenKind assign_kind, std::unique_ptr<ExpressionList> lexp_list,
      std::unique_ptr<AssignableList> rvalue_list) {
    return std::unique_ptr<AssignmentStatement>(new AssignmentStatement(
        assign_kind, std::move(lexp_list), std::move(rvalue_list), fn_pos_()));
  }

  inline std::unique_ptr<ExpressionList> NewExpressionList(
      std::vector<std::unique_ptr<Expression>> exps) {
    return std::unique_ptr<ExpressionList>(new ExpressionList(std::move(exps),
                                                              fn_pos_()));
  }

  inline std::unique_ptr<Expression> NewEllipsisExpression(
      std::unique_ptr<Expression> expr) {
    return std::unique_ptr<Expression>(new EllipsisExpression(std::move(expr),
                                                              fn_pos_()));
  }

  inline std::unique_ptr<StatementList> NewStatementList(
      std::vector<std::unique_ptr<Statement>> stmt_list) {
    return std::unique_ptr<StatementList>(new StatementList(
        std::move(stmt_list), fn_pos_()));
  }

  inline std::unique_ptr<FunctionCall> NewFunctionCall(
      std::unique_ptr<Expression> func_exp,
      std::unique_ptr<AssignableList> rvalue_list) {
    return std::unique_ptr<FunctionCall>(new FunctionCall(
        std::move(func_exp), std::move(rvalue_list), fn_pos_()));
  }

  inline std::unique_ptr<ExpressionStatement> NewExpressionStatement(
      std::unique_ptr<Expression> exp_stmt) {
    return std::unique_ptr<ExpressionStatement>(new ExpressionStatement(
        std::move(exp_stmt), fn_pos_()));
  }

  inline std::unique_ptr<Statement> NewBlock(
      std::unique_ptr<StatementList> stmt_list) {
    return std::unique_ptr<Statement>(new Block(
        std::move(stmt_list), fn_pos_()));
  }

  inline std::unique_ptr<BreakStatement> NewBreakStatement() {
    return std::unique_ptr<BreakStatement>(new BreakStatement(fn_pos_()));
  }

  inline std::unique_ptr<ContinueStatement> NewContinueStatement() {
    return std::unique_ptr<ContinueStatement>(new ContinueStatement(fn_pos_()));
  }

  inline std::unique_ptr<DefaultStatement> NewDefaultStatement(
      std::unique_ptr<Block> block) {
    return std::unique_ptr<DefaultStatement>(new DefaultStatement(
        std::move(block), fn_pos_()));
  }

  inline std::unique_ptr<IfStatement> NewIfStatement(
      std::unique_ptr<Expression> exp,
      std::unique_ptr<Statement> then_block,
      std::unique_ptr<Statement> else_block) {
    return std::unique_ptr<IfStatement>(new IfStatement(
        std::move(exp), std::move(then_block), std::move(else_block),
        fn_pos_()));
  }

  inline std::unique_ptr<WhileStatement> NewWhileStatement(
      std::unique_ptr<Expression> exp,
      std::unique_ptr<Statement> block) {
    return std::unique_ptr<WhileStatement>(new WhileStatement(
        std::move(exp), std::move(block), fn_pos_()));
  }

  inline std::unique_ptr<SwitchStatement> NewSwitchStatement(
      std::unique_ptr<Expression> exp,
      std::vector<std::unique_ptr<CaseStatement>>&& case_list,
      std::unique_ptr<DefaultStatement> default_stmt) {
    return std::unique_ptr<SwitchStatement>(new SwitchStatement(
        std::move(exp), std::move(case_list), std::move(default_stmt),
        fn_pos_()));
  }

  inline std::unique_ptr<ForInStatement> NewForInStatement(
      std::unique_ptr<ExpressionList> exp_list,
      std::unique_ptr<ExpressionList> test_list,
      std::unique_ptr<Statement> block) {
    return std::unique_ptr<ForInStatement>(new ForInStatement(
        std::move(exp_list), std::move(test_list), std::move(block),
        fn_pos_()));
  }

  inline std::unique_ptr<CaseStatement> NewCaseStatement(
      std::unique_ptr<ExpressionList> exp_list,
      std::unique_ptr<Block> block) {
    return std::unique_ptr<CaseStatement>(new CaseStatement(
        std::move(exp_list), std::move(block), fn_pos_()));
  }

  inline std::unique_ptr<DelStatement> NewDelStatement(
      std::unique_ptr<ExpressionList> exp_list) {
    return std::unique_ptr<DelStatement>(new DelStatement(
        std::move(exp_list), fn_pos_()));
  }

  inline std::unique_ptr<AliasDeclaration> NewAliasDeclaration(
      std::unique_ptr<SimpleCmd>&& cmd, std::unique_ptr<Identifier>&& name) {
    return std::unique_ptr<AliasDeclaration>(new AliasDeclaration(
      std::move(cmd), std::move(name), fn_pos_()));
  }

  inline std::unique_ptr<CmdPiece> NewCmdPiece(const Token& token) {
    return std::unique_ptr<CmdPiece>(new CmdPiece(token, fn_pos_()));
  }

  inline std::unique_ptr<SimpleCmd> NewSimpleCmd(
      std::vector<std::unique_ptr<AstNode>>&& pieces) {
    return std::unique_ptr<SimpleCmd>(new SimpleCmd(
        std::move(pieces), fn_pos_()));
  }

  inline std::unique_ptr<FilePathCmd> NewFilePathCmd(
      std::vector<std::unique_ptr<AstNode>>&& pieces) {
    return std::unique_ptr<FilePathCmd>(new FilePathCmd(
        std::move(pieces), fn_pos_()));
  }

  inline std::unique_ptr<CmdIoRedirect> NewCmdIoRedirect(
      std::unique_ptr<Literal> integer, std::unique_ptr<FilePathCmd> fp_cmd,
      TokenKind kind, bool all) {
    return std::unique_ptr<CmdIoRedirect>(new CmdIoRedirect(
        std::move(integer), std::move(fp_cmd), kind, all, fn_pos_()));
  }

  inline std::unique_ptr<CmdIoRedirectList> NewCmdIoRedirectList(
      std::unique_ptr<Cmd> cmd,
      std::vector<std::unique_ptr<CmdIoRedirect>>&& io_list) {
    return std::unique_ptr<CmdIoRedirectList>(new CmdIoRedirectList(
        std::move(cmd), std::move(io_list), fn_pos_()));
  }

  inline std::unique_ptr<CmdPipeSequence> NewCmdPipeSequence(
      std::vector<std::unique_ptr<Cmd>>&& cmds) {
    return std::unique_ptr<CmdPipeSequence>(new CmdPipeSequence(std::move(cmds),
                                                                fn_pos_()));
  }

  inline std::unique_ptr<CmdAndOr> NewCmdAndOr(
      TokenKind token_kind, std::unique_ptr<Cmd> cmd_left,
      std::unique_ptr<Cmd> cmd_right) {
    return std::unique_ptr<CmdAndOr>(new CmdAndOr(
        token_kind, std::move(cmd_left),  std::move(cmd_right), fn_pos_()));
  }

  inline std::unique_ptr<CmdFull> NewCmdFull(std::unique_ptr<Cmd> cmd,
                                             bool background) {
    return std::unique_ptr<CmdFull>(new CmdFull(
        std::move(cmd),  background, fn_pos_()));
  }

  inline std::unique_ptr<CmdExpression> NewCmdExpression(
      std::unique_ptr<Cmd> cmd) {
    return std::unique_ptr<CmdExpression>(new CmdExpression(
        std::move(cmd), fn_pos_()));
  }

  inline std::unique_ptr<FunctionParam> NewFunctionParam(
      std::unique_ptr<Identifier> id, std::unique_ptr<AssignableValue> value,
      bool variadic) {
    return std::unique_ptr<FunctionParam>(new FunctionParam(
        std::move(id), std::move(value), variadic, fn_pos_()));
  }

  inline std::unique_ptr<FunctionDeclaration> NewFunctionDeclaration(
      std::vector<std::unique_ptr<FunctionParam>>&& params,
      std::unique_ptr<Identifier> name,
      std::unique_ptr<Block> block, Position pos) {
    return std::unique_ptr<FunctionDeclaration>(new FunctionDeclaration(
        std::move(params), std::move(name), std::move(block), pos));
  }

  inline std::unique_ptr<FunctionExpression> NewFunctionExpression(
      std::vector<std::unique_ptr<FunctionParam>>&& params,
      std::unique_ptr<Block> block, Position pos) {
    return std::unique_ptr<FunctionExpression>(new FunctionExpression(
        std::move(params), std::move(block), pos));
  }

  inline std::unique_ptr<ArrayInstantiation> NewArrayInstantiation(
      std::unique_ptr<AssignableList> elements) {
    return std::unique_ptr<ArrayInstantiation>(new ArrayInstantiation(
        std::move(elements), fn_pos_()));
  }

  template<class T>
  inline std::unique_ptr<AssignableValue> NewAssignableValue(
      std::unique_ptr<T>&& value) {
    return std::unique_ptr<AssignableValue>(new AssignableValue(
        std::move(value), fn_pos_()));
  }

  inline std::unique_ptr<AssignableList> NewAssignableList(
      std::vector<std::unique_ptr<AssignableValue>>&& nodes) {
    return std::unique_ptr<AssignableList>(new AssignableList(
        std::move(nodes), fn_pos_()));
  }

  inline std::unique_ptr<KeyValue> NewKeyValue(
      std::unique_ptr<Expression> key, std::unique_ptr<AssignableValue> value) {
    return std::unique_ptr<KeyValue>(new KeyValue(
        std::move(key), std::move(value), fn_pos_()));
  }

  inline std::unique_ptr<DictionaryInstantiation> NewDictionaryInstantiation(
      std::vector<std::unique_ptr<KeyValue>>&& key_value_list) {
    return std::unique_ptr<DictionaryInstantiation>(new DictionaryInstantiation(
        std::move(key_value_list), fn_pos_()));
  }

  inline std::unique_ptr<ReturnStatement> NewReturnStatement(
      std::unique_ptr<AssignableList> assign_list) {
    return std::unique_ptr<ReturnStatement>(new ReturnStatement(
        std::move(assign_list), fn_pos_()));
  }

  inline std::unique_ptr<CmdDeclaration> NewCmdDeclaration(
      std::unique_ptr<Identifier> id, std::unique_ptr<Block> block) {
    return std::unique_ptr<CmdDeclaration>(new CmdDeclaration(
        std::move(id), std::move(block), fn_pos_()));
  }

  inline std::unique_ptr<SubShell> NewSubShell(std::unique_ptr<Block> block) {
    return std::unique_ptr<SubShell>(new SubShell(std::move(block), fn_pos_()));
  }

  inline std::unique_ptr<Slice> NewSlice(std::unique_ptr<Expression> start_exp,
                                         std::unique_ptr<Expression> end_exp) {
    return std::unique_ptr<Slice>(
        new Slice(std::move(start_exp), std::move(end_exp), fn_pos_()));
  }

  inline std::unique_ptr<ClassDeclList> NewClassDeclList(
      std::vector<std::unique_ptr<Declaration>> decl_list) {
    return std::unique_ptr<ClassDeclList>(new ClassDeclList(
      std::move(decl_list), fn_pos_()));
  }

  inline std::unique_ptr<ClassBlock> NewClassBlock(
      std::unique_ptr<ClassDeclList> decl_list) {
    return std::unique_ptr<ClassBlock>(new ClassBlock(
      std::move(decl_list), fn_pos_()));
  }

  inline std::unique_ptr<ClassDeclaration> NewClassDeclaration(
      std::unique_ptr<Identifier> name,
      std::unique_ptr<Identifier> id_parent,
      std::vector<std::unique_ptr<Identifier>> interfaces,
      std::unique_ptr<ClassBlock> block, bool is_final) {
    return std::unique_ptr<ClassDeclaration>(new ClassDeclaration(
      std::move(name), std::move(id_parent), std::move(interfaces),
      std::move(block), is_final, fn_pos_()));
  }

  inline std::unique_ptr<DeferStatement> NewDeferStatement(
      std::unique_ptr<Statement> stmt) {
    return std::unique_ptr<DeferStatement>(new DeferStatement(std::move(stmt),
        fn_pos_()));
  }

  inline std::unique_ptr<CmdValueExpr> NewCmdValueExpr(
     std::unique_ptr<Expression> expr, bool has_blank_space, bool is_iterator) {
    return std::unique_ptr<CmdValueExpr>(new CmdValueExpr(std::move(expr),
        has_blank_space, is_iterator, fn_pos_()));
  }

  inline std::unique_ptr<ImportStatement> NewImportStatement(
      ImportStatement::From from, ImportStatement::Import import,
      std::unique_ptr<Identifier> as) {
    return std::unique_ptr<ImportStatement>(new ImportStatement(std::move(from),
       std::move(import), std::move(as), fn_pos_()));
  }

  inline std::unique_ptr<ImportStatement> NewImportStatement(
      ImportStatement::Import import, std::unique_ptr<Identifier> as) {
    return std::unique_ptr<ImportStatement>(new ImportStatement(
       std::move(import), std::move(as), fn_pos_()));
  }

 private:
  std::function<Position()> fn_pos_;
};

}
}
