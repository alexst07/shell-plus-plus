#include "assign_executor.h"

#include <string>
#include <boost/variant.hpp>

#include "expr_executor.h"

namespace setti {
namespace internal {

void AssignExecutor::Exec(AstNode* node) {
  AssignmentStatement* assign_node = static_cast<AssignmentStatement*>(node);
}

std::vector<std::unique_ptr<Object>> AssignExecutor::ExecAssignableList(
    AstNode* node) {
  AssignableList* assign_list_node = static_cast<AssignableList*>(node);

  std::vector<std::unique_ptr<Object>> obj_vec;

  for (AstNode* value: assign_list_node->children()) {
    obj_vec.push_back(std::move(ExecAssignable(value)));
  }
}

std::unique_ptr<Object> AssignExecutor::ExecAssignable(AstNode* node) {
  if (AstNode::IsExpression(node->type())) {
    ExpressionExecutor expr_exec(this);
    return expr_exec.Exec(node);
  }
}

}
}
