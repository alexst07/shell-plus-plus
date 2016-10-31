#ifndef SETI_ASSIGN_EXECUTOR_H
#define SETI_ASSIGN_EXECUTOR_H

#include <string>
#include <memory>
#include <vector>
#include <tuple>

#include "ast/ast.h"
#include "ast/obj_type.h"
#include "executor.h"

namespace setti {
namespace internal {

class AssignExecutor: public Executor {
 public:
  // Entry point to execute assign operations
  void Exec(AstNode* node);

  // Executes assignable values, that could be a list
  // with functions or expressions
  std::unique_ptr<Object> ExecAssignable(AstNode* node);

  // Executes assignable list, it can be function or expression
  std::vector<std::unique_ptr<Object>> ExecAssignableList(AstNode* node);
};

}
}

#endif  // SETI_ASSIGN_EXECUTOR_H


