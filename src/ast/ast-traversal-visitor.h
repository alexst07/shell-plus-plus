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

class AstTraversalVisitor: public AstVisitor {
  int level_;
 public:
  void virtual VisitBinaryOperation(BinaryOperation* bin_op) {
    level_++;
    bin_op->left()->Accept(this);
    bin_op->right()->Accept(this);
    level_--;
    std::cout << "level: " << level_;
    std::cout << "VisitBinaryOperation: "
              << Token::name(bin_op->token_kind()) << "\n";

  }

  void virtual VisitLiteral(Literal* lit_exp) {
    std::cout << "level: " << level_;
    std::cout << "VisitLiteral: "<< lit_exp->value() << "\n";
  }

  void Visit(AstNode *node) {
    level_ = 0;
    node->Accept(this);
  }
};

}
}
#endif  // SETTI_AST_TRAVERSAL_VISITOR_H


