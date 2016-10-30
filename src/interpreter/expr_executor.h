#ifndef SETI_EXPR_EXECUTOR_H
#define SETI_EXPR_EXECUTOR_H

#include <string>
#include <memory>
#include <unordered_map>
#include <tuple>

#include "ast/ast.h"
#include "ast/obj_type.h"
#include "executor.h"

namespace setti {
namespace internal {

class ExpressionExecutor: public Executor {
 public:
  // Entry point to execute expression
  std::unique_ptr<Object> Exec(AstNode* node);

  // Executes literal const and return an object with its value
  std::unique_ptr<Object> ExecLiteral(AstNode* node);

};

}
}

#endif  // SETI_EXPR_EXECUTOR_H


