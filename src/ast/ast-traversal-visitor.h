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
 public:
  void virtual VisitBinaryOperation(BinaryOperation* bin_op) {
    std::cout << "VisitBinaryOperation: "
              << Token::name(bin_op->token_kind()) << "\n";
  }

  void virtual VisitLiteral(Literal* lit_exp) {
    std::cout << "VisitLiteral: "<< lit_exp->token() << "\n";
  }

  void Visit(AstNode *node) {
    node->Accept(this);
  }
};

}
}
#endif  // SETTI_AST_TRAVERSAL_VISITOR_H


