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

  void Level() {
    for (int i = 0; i < level_-1; i++) {
      std::cout << " │";
    }

    std::cout << " ├";
    std::cout << "─";
  }

 public:
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
    std::cout << "<assign id: " << assign->id()->name() << " kind: "
              << static_cast<int>(assign->assign_kind()) << ">\n";
    level_++;
    assign->exp()->Accept(this);
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

  void Visit(AstNode *node) {
    level_ = 0;
    node->Accept(this);
  }
};

}
}
#endif  // SETTI_AST_TRAVERSAL_VISITOR_H


