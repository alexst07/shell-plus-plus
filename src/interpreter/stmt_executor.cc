#include "stmt_executor.h"

#include <string>
#include <boost/variant.hpp>

#include "assign_executor.h"
#include "expr_executor.h"

namespace setti {
namespace internal {

void StmtListExecutor::Exec(AstNode* node) {
  StatementList* stmt_list = static_cast<StatementList*>(node);

  for (AstNode* stmt: stmt_list->children()) {
    StmtExecutor stmt_exec(this, symbol_table_stack());
    stmt_exec.Exec(stmt);
  }
}

void StmtExecutor::Exec(AstNode* node) {
  switch (node->type()) {
    case AstNode::NodeType::kAssignmentStatement: {
      AssignExecutor exec(this, symbol_table_stack());
      return exec.Exec(node);
    } break;

    case AstNode::NodeType::kExpressionStatement: {
      ExpressionExecutor exec(this, symbol_table_stack());
      exec.Exec(static_cast<ExpressionStatement&>(*node).exp());
      return;
    } break;

  }
}

}
}
