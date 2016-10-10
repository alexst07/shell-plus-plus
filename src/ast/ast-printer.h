#ifndef SETTI_AST_TRAVERSAL_VISITOR_H
#define SETTI_AST_TRAVERSAL_VISITOR_H

#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <boost/format.hpp>

#include "ast.h"

namespace setti {
namespace internal {

class AstPrinter: public AstVisitor {
  int level_;
  bool inside_cmd_;

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

  void virtual VisitIdentifier(Identifier* id) {
    Level();
    std::cout << "<identifier name: "<< id->name() << ">\n";
  }

  void virtual VisitLiteral(Literal* lit_exp) {
    Level();
    std::cout << "<literal value: "<< lit_exp->value() << ">\n";
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

    level_++;
    assign->rexp_list()->Accept(this);
    level_--;
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
    std::cout << "<function_exp_list>\n";
    level_++;
    fn->exp_list()->Accept(this);
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

    case_stmt->exp()->Accept(this);
    level_--;
  }

  void virtual VisitBreakStatement(BreakStatement* pbreak) {
    Level();
    std::cout << "<break>\n";
  }

  void virtual VisitDefaultStatement(DefaultStatement* default_stmt) {
    Level();
    std::cout << "<default>\n";
  }

  void virtual VisitCmdPiece(CmdPiece* cmd_piece) {
    std::cout << cmd_piece->cmd_str();
    if (cmd_piece->blank_after()) {
      std::cout << " ";
    }
  }

  void virtual VisitSimpleCmd(SimpleCmd* cmd) {
    inside_cmd_ = true;

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

  void virtual VisitCmdIoRedirect(CmdIoRedirect* cmd_io) {
    Level();
    std::cout << "<io_redirect: ";
    cmd_io->cmd()->Accept(this);
    if (cmd_io->has_integer()) {
      cmd_io->integer()->Accept(this);
    }

    if (cmd_io->all()) {
      std::cout << "&";
    }

    cmd_io->file_path_cmd()->Accept(this);
    std::cout << ">\n";
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
    std::cout << "<block>\n";
    level_++;
    switch_stmt->block()->Accept(this);
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
    cmd_pipe->cmd_left()->Accept(this);
    cmd_pipe->cmd_right()->Accept(this);
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
};

}
}
#endif  // SETTI_AST_TRAVERSAL_VISITOR_H


