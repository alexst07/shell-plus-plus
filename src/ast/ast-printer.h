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

#ifndef SHPP_AST_TRAVERSAL_VISITOR_H
#define SHPP_AST_TRAVERSAL_VISITOR_H

#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <boost/format.hpp>

#include "ast.h"

namespace shpp {
namespace internal {

class AstPrinter: public AstVisitor {
 public:
  AstPrinter() {
    inside_scope_ = false;
    inside_cmd_ = false;
  }

 private:
  int level_;
  bool inside_cmd_;
  bool inside_scope_;

  void Level() {
    for (int i = 0; i < level_-1; i++) {
      std::cout << " │";
    }

    std::cout << " ├";
    std::cout << "─";
  }

 public:
  void virtual VisitStatementList(StatementList * stmt_list) {
    auto vec = stmt_list->children();
    int i = 0;
    for (const auto stmt: vec) {
      Level();
      std::cout << "<stmt i: " << i << ">\n";
      level_++;
      stmt->Accept(this);
      level_--;
      i++;
    }
  }

  void virtual VisitBinaryOperation(BinaryOperation* bin_op) {
    Level();
    std::cout << "<bin_op type: "
              << Token::name(bin_op->kind()) << ">\n";
    level_++;
    bin_op->left()->Accept(this);
    bin_op->right()->Accept(this);
    level_--;
  }

  void virtual VisitUnaryOperation(UnaryOperation* un_op) {
    Level();
    std::cout << "<un_op type: "
              << Token::name(un_op->kind()) << ">\n";
    level_++;
    un_op->exp()->Accept(this);
    level_--;
  }

  void virtual VisitNotExpression(NotExpression* not_expr) {
    Level();
    std::cout << "<not type: "
              << Token::name(not_expr->kind()) << ">\n";
    level_++;
    not_expr->exp()->Accept(this);
    level_--;
  }

  void virtual VisitIdentifier(Identifier* id) {
    if (inside_scope_) {
      std::cout << id->name();
      std::cout << "::";
      if (id->has_scope()) {

        id->scope()->Accept(this);
      }
      return;
    }

    Level();
    std::cout << "<identifier name: "<< id->name();
    if (id->has_scope()) {
      std::cout << " scope: ";
      id->scope()->Accept(this);
    }
    std::cout << ">\n";
  }

  void virtual VisitPackageScope(PackageScope* scope) {
    inside_scope_ = true;
    scope->id()->Accept(this);
    inside_scope_ = false;
  }

  void virtual VisitLiteral(Literal* lit_exp) {
    Level();
    std::cout << "<literal value: "<< lit_exp->value() << ">\n";
  }

  void virtual VisitGlob(Glob* glob) {
    inside_cmd_ = true;
    Level();
    std::cout << "<glob: ";
    auto vec = glob->children();

    for (const auto c: vec) {
      c->Accept(this);
    }
    std::cout << ">\n";
  }

  void virtual VisitAttribute(Attribute* att) {
    Level();
    std::cout << "<attribute>\n";
    level_++;
    Level();
    std::cout << "<expression>\n";
    level_++;
    att->exp()->Accept(this);
    level_--;
    Level();
    std::cout << "<id>\n";
    level_++;
    att->id()->Accept(this);
    level_--;
    level_--;
  }

  void virtual VisitArray(Array* arr) {
    Level();
    std::cout << "<array>\n";
    level_++;
    Level();
    std::cout << "<expression>\n";
    level_++;
    arr->arr_exp()->Accept(this);
    level_--;
    Level();
    std::cout << "<index>\n";
    level_++;
    arr->index_exp()->Accept(this);
    level_--;
    level_--;
  }

  void virtual VisitAssignmentStatement(AssignmentStatement* assign) {
    Level();
    std::cout << "<assign>\n";
    level_++;
    assign->lexp_list()->Accept(this);
    level_--;

    Level();
    std::cout << "<kind: " << static_cast<int>(assign->assign_kind()) << ">\n";

    if (assign->has_rvalue()) {
      level_++;
      assign->rvalue_list()->Accept(this);
      level_--;
    }
  }

  void virtual VisitExpressionList(ExpressionList* list) {
    Level();
    std::cout << "<expression_list>\n";
    level_++;
    auto vec = list->children();
    int i = 0;
    for (const auto c: vec) {
      Level();
      std::cout << "<expression i: " << i << ">\n";
      level_++;
      c->Accept(this);
      level_--;
      i++;
    }
    level_--;
  }

  void virtual VisitArgumentsList(ArgumentsList *list) {
    Level();
    std::cout << "<arguments_list>\n";
    level_++;
    auto vec = list->children();
    int i = 0;
    for (const auto c: vec) {
      Level();
      std::cout << "<arg i: " << i << ">\n";
      level_++;
      c->Accept(this);
      level_--;
      i++;
    }
    level_--;
  }

  void virtual VisitArgument(Argument *arg) {
    level_++;

    if (arg->has_key()) {
      Level();
      std::cout << "<key>" << arg->key() << "</key>\n";
    }

    arg->arg()->Accept(this);

    level_--;
  }

  void virtual VisitFunctionCall(FunctionCall* fn) {
    Level();
    std::cout << "<function_call>\n";
    level_++;

    Level();
    std::cout << "<function_exp_caller>\n";
    level_++;
    fn->func_exp()->Accept(this);
    level_--;

    Level();
    std::cout << "<function_argument_list>\n";
    level_++;
    fn->args_list()->Accept(this);
    level_--;
    level_--;
  }

  void virtual VisitIfStatement(IfStatement* if_stmt) {
    Level();
    std::cout << "<if>\n";
    level_++;

    Level();
    std::cout << "<condition>\n";
    level_++;
    if_stmt->exp()->Accept(this);
    level_--;

    Level();
    std::cout << "<then>\n";
    level_++;
    if_stmt->then_block()->Accept(this);
    level_--;

    if (if_stmt->has_else()) {
      Level();
      std::cout << "<else>\n";
      level_++;
      if_stmt->else_block()->Accept(this);
      level_--;
    }

    level_--;
  }

  void virtual VisitWhileStatement(WhileStatement* while_stmt) {
    Level();
    std::cout << "<while>\n";
    level_++;

    Level();
    std::cout << "<condition>\n";
    level_++;
    while_stmt->exp()->Accept(this);
    level_--;

    Level();
    std::cout << "<do>\n";
    level_++;
    while_stmt->block()->Accept(this);
    level_--;

    level_--;
  }

  void virtual VisitBlock(Block* block) {
    Level();
    std::cout << "<block>\n";
    level_++;

    block->stmt_list()->Accept(this);
    level_--;
  }

  void virtual VisitExpressionStatement(ExpressionStatement* exp_stmt) {
    Level();
    std::cout << "<exp_stmt>\n";
    level_++;

    exp_stmt->exp()->Accept(this);
    level_--;
  }

  void virtual VisitCaseStatement(CaseStatement* case_stmt) {
    Level();
    std::cout << "<case_stmt>\n";
    level_++;

    case_stmt->exp_list()->Accept(this);
    case_stmt->block()->Accept(this);

    level_--;
  }

  void virtual VisitDelStatement(DelStatement* del_stmt) {
    Level();
    std::cout << "<del_stmt>\n";
    level_++;

    del_stmt->exp_list()->Accept(this);

    level_--;
  }

  void virtual VisitBreakStatement(BreakStatement* pbreak) {
    Level();
    std::cout << "<break>\n";
  }

  void virtual VisitDefaultStatement(DefaultStatement* default_stmt) {
    Level();
    std::cout << "<default>\n";
    level_++;
    default_stmt->block()->Accept(this);
    level_--;
  }

  void virtual VisitCmdPiece(CmdPiece* cmd_piece) {
    std::cout << cmd_piece->cmd_str();
    if (cmd_piece->blank_after()) {
      std::cout << " ";
    }
  }

  void virtual VisitAliasDeclaration(AliasDeclaration* alias) {
    Level();
    std::cout << "<alias name: ";
    alias->name()->Accept(this);
    std::cout << " cmd: ";
    alias->cmd()->Accept(this);
  }

  void virtual VisitSimpleCmd(SimpleCmd* cmd) {
    inside_cmd_ = true;
    Level();
    std::cout << "<cmd: ";
    auto vec = cmd->children();

    for (const auto c: vec) {
      c->Accept(this);
    }
    std::cout << ">\n";
  }

  void virtual VisitFilePathCmd(FilePathCmd* fp_cmd) {
    Level();
    std::cout << "<cmd_path: ";
    auto vec = fp_cmd->children();

    for (const auto c: vec) {
      c->Accept(this);
    }

    std::cout << ">\n";
  }

  void virtual VisitCmdIoRedirectList(CmdIoRedirectList *cmd_io_list) {
    Level();
    std::cout << "<io_redirect_list: ";

    cmd_io_list->cmd()->Accept(this);
    auto vec = cmd_io_list->children();

    for (const auto c: vec) {
      c->Accept(this);
    }

    std::cout << ">\n";
  }

  void virtual VisitCmdIoRedirect(CmdIoRedirect* cmd_io) {
    if (cmd_io->has_integer()) {
      cmd_io->integer()->Accept(this);
    }

    if (cmd_io->all()) {
      std::cout << "all(&) ";
    }

    std::cout << "kind: " << token_value_str[int(cmd_io->kind())] << "\n";

    cmd_io->file_path_cmd()->Accept(this);
  }

  void virtual VisitSwitchStatement(SwitchStatement* switch_stmt) {
    Level();
    std::cout << "<switch>\n";
    level_++;

    if (switch_stmt->has_exp()) {
       Level();
      std::cout << "<exp>\n";
      level_++;
      switch_stmt->exp()->Accept(this);
      level_--;
    }

    Level();
    std::cout << "<block_switch>\n";
    level_++;
    std::vector<CaseStatement*> case_list = switch_stmt->case_list();
    for (auto& c: case_list) {
      c->Accept(this);
    }

    if (switch_stmt->has_default()) {
      switch_stmt->default_stmt()->Accept(this);
    }

    level_--;

    level_--;
  }

  void virtual VisitForInStatement(ForInStatement* for_in_stmt) {
    Level();
    std::cout << "<for_in>\n";
    level_++;

    Level();
    std::cout << "<exp_list>\n";
    level_++;
    for_in_stmt->exp_list()->Accept(this);
    level_--;

    Level();
    std::cout << "<test_list>\n";
    level_++;
    for_in_stmt->test_list()->Accept(this);
    level_--;

    Level();
    std::cout << "<block>\n";
    level_++;
    for_in_stmt->block()->Accept(this);
    level_--;

    level_--;
  }

  void Visit(AstNode *node) {
    level_ = 0;
    inside_cmd_ = false;
    node->Accept(this);
  }

  void virtual VisitCmdPipeSequence(CmdPipeSequence* cmd_pipe) {
    Level();
    std::cout << "<command pipe>\n";
    level_++;
    for (auto& e: cmd_pipe->cmds()) {
      e->Accept(this);
    }
    level_--;
  }

  void virtual VisitCmdValueExpr(CmdValueExpr* cmd) {
    Level();
    std::cout << "<cmd_expr_value has_blank_space=" << cmd->blank_after()
              << ">";
    level_++;
    cmd->expr()->Accept(this);
    level_--;
  }

  void virtual VisitCmdAndOr(CmdAndOr* cmd_and_or) {
    Level();
    std::cout << "<command_and_or type: "
              << Token::name(cmd_and_or->kind()) << ">\n";
    level_++;
    cmd_and_or->cmd_left()->Accept(this);
    cmd_and_or->cmd_right()->Accept(this);
    level_--;
  }

  void virtual VisitCmdFull(CmdFull* cmd_full) {
    Level();
    std::cout << "<command_full background: "
              << (cmd_full->background()? "true": "false") << ">\n";
    level_++;
    cmd_full->cmd()->Accept(this);
    level_--;
  }

  void virtual VisitCmdExpression(CmdExpression* cmd) {
    cmd->cmd()->Accept(this);
  }

  void virtual VisitFunctionParam(FunctionParam* func_param) {
    Level();
    std::cout << "<function_param variadic: "
              << (func_param->variadic()? "true": "false") << ">\n";
    level_++;
    func_param->id()->Accept(this);

    if (func_param->has_value()) {
      func_param->value()->Accept(this);
    }
    level_--;
  }

  void virtual VisitFunctionDeclaration(FunctionDeclaration* func_decl) {
    Level();
    std::cout << "<function variadic:"
              << (func_decl->variadic()? "true": "false") << "declaration: yes>\n";
    level_++;

    func_decl->name()->Accept(this);

    auto vec = func_decl->children();

    for (const auto c: vec) {
      c->Accept(this);
    }

    if (func_decl->has_block()) {
      func_decl->block()->Accept(this);
    }

    level_--;
  }

    void virtual VisitFunctionExpression(FunctionExpression* func_decl) {
    Level();
    std::cout << "<function variadic:"
              << (func_decl->variadic()? "true": "false") << ">\n";
    level_++;

    auto vec = func_decl->children();

    for (const auto c: vec) {
      c->Accept(this);
    }

    if (func_decl->has_block()) {
      func_decl->block()->Accept(this);
    }

    level_--;
  }

  void virtual VisitArrayInstantiation(ArrayInstantiation* array) {
    Level();
    std::cout << "<array_instantiation>\n";
    if (array->valid_elements()) {
      level_++;
      array->assignable_list()->Accept(this);
      level_--;
    }
  }

  void virtual VisitLetExpression(LetExpression* let) {
    Level();
    std::cout << "<let>\n";
    level_++;
    let->assign()->Accept(this);
    level_--;
    Level();
    std::cout << "</let>\n";
  }

  void virtual VisitAssignableValue(AssignableValue* value) {
    Level();
    std::cout << "<AssignableValue>\n";
    level_++;
    value->value()->Accept(this);
    level_--;
  }

  void virtual VisitKeyValue(KeyValue* key_value) {
    Level();
    std::cout << "<key_value>\n";
    level_++;
    std::cout << "<key>\n";
    level_++;
    key_value->key()->Accept(this);
    level_--;
    std::cout << "<value>\n";
    level_++;
    key_value->value()->Accept(this);
    level_--;
    level_--;
  }

  void virtual VisitDictionaryInstantiation(DictionaryInstantiation* dic) {
    Level();
    std::cout << "<dictionary_instantiation>\n";
    level_++;

    auto vec = dic->children();

    for (const auto c: vec) {
      c->Accept(this);
    }
    level_--;
  }

  void virtual VisitReturnStatement(ReturnStatement* ret) {
    Level();
    std::cout << "<return>\n";
    level_++;

    if (!ret->is_void()) {
      ret->assign_list()->Accept(this);
    }

    level_--;
  }

  void virtual VisitCmdDeclaration(CmdDeclaration* cmd_decl) {
    Level();
    std::cout << "<cmd_declaration>\n";
    level_++;
    cmd_decl->id()->Accept(this);
    cmd_decl->block()->Accept(this);
    level_--;
  }

  void virtual VisitSubShell(SubShell* sub_shell) {
    Level();
    std::cout << "<sub_shell self-process:"
      << sub_shell->self_process()
      << ">\n";
    level_++;
    sub_shell->block()->Accept(this);
    level_--;
  }

  void virtual VisitAssignableList(AssignableList* assign_list) {
    for (auto& e: assign_list->children()) {
      e->Accept(this);
    }
  }

  void virtual VisitSlice(Slice *slice) {
    Level();
    std::cout << "<slice>\n";
    level_++;
    if (slice->has_start_exp()) {
      slice->start_exp()->Accept(this);
    }

    if (slice->has_end_exp()) {
      slice->end_exp()->Accept(this);
    }
    level_--;
  }

  void virtual VisitClassDeclList(ClassDeclList* class_decl_list) {
    if (!class_decl_list->IsEmpty()) {
      auto vec = class_decl_list->children();

      for (const auto c: vec) {
        c->Accept(this);
      }
    }
  }

  void virtual VisitDeferStatement(DeferStatement* defer) {
    Level();
    std::cout << "<defer>\n";
    level_++;
    defer->stmt()->Accept(this);
    level_--;
    Level();
    std::cout << "<end>\n";
  }

  void virtual VisitImportStatement(ImportStatement* import) {
    Level();
    std::cout << "<import>\n";
    level_++;

    if (import->is_import_path()) {
      Level();
      std::cout << "<path>\n";
      level_++;
      import->import<Literal>()->Accept(this);
      level_--;
    } else {
      Level();
      std::cout << "<id>\n";
      level_++;
      import->import<Identifier>()->Accept(this);
      level_--;
    }

    if (import->has_as()) {
      Level();
      std::cout << "<as>\n";
      level_++;
      import->as()->Accept(this);
    }
  }

  void virtual VisitClassBlock(ClassBlock* class_block) {
    Level();
    std::cout << "<begin>\n";
    level_++;
    class_block->decl_list()->Accept(this);
    level_--;
    Level();
    std::cout << "<end>\n";
  }

  void virtual VisitClassDeclaration(ClassDeclaration* class_decl) {
    Level();

    std::cout << "<class name: " << class_decl->name()->name() << ">\n";

    if (class_decl->has_parent()) {
      Level();
      std::cout << "<parent>\n";
      level_++;
      class_decl->parent()->Accept(this);
      level_--;
      Level();
      std::cout << "</parent>\n";
    }

    if (class_decl->has_interfaces()) {
      Level();
      std::cout << "<interfaces>\n";
      level_++;
      class_decl->interfaces()->Accept(this);
      level_--;
      Level();
      std::cout << "</interfaces>\n";
    }

    level_++;
    class_decl->block()->Accept(this);
    level_--;
  }

  void virtual VisitInterfaceDeclList(InterfaceDeclList* iface_decl_list) {
    if (!iface_decl_list->IsEmpty()) {
      auto vec = iface_decl_list->children();

      for (const auto c: vec) {
        c->Accept(this);
      }
    }
  }

  void virtual VisitInterfaceBlock(InterfaceBlock* iface_block) {
    Level();
    std::cout << "<begin>\n";
    level_++;
    iface_block->decl_list()->Accept(this);
    level_--;
    Level();
    std::cout << "<end>\n";
  }

  void virtual VisitInterfaceDeclaration(InterfaceDeclaration* iface_decl) {
    Level();

    std::cout << "<interface name: " << iface_decl->name()->name() << ">\n";

    if (iface_decl->has_interfaces()) {
      Level();
      std::cout << "<interfaces>\n";
      level_++;
      iface_decl->interfaces()->Accept(this);
      level_--;
      Level();
      std::cout << "</interfaces>\n";
    }

    level_++;
    iface_decl->block()->Accept(this);
    level_--;
  }
};

}
}
#endif  // SETTI_AST_TRAVERSAL_VISITOR_H
