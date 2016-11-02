#ifndef SETI_EXPR_EXECUTOR_H
#define SETI_EXPR_EXECUTOR_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <tuple>

#include "ast/ast.h"
#include "ast/obj_type.h"
#include "executor.h"

namespace setti {
namespace internal {

class AssignableListExecutor: public Executor {
 public:
  AssignableListExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  std::vector<std::unique_ptr<Object>> Exec(AstNode* node);

  std::unique_ptr<Object> ExecAssignable(AstNode* node);
};

class ExpressionExecutor: public Executor {
 public:
  ExpressionExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  // Entry point to execute expression
  std::unique_ptr<Object> Exec(AstNode* node);

  // Executes literal const and return an object with its value
  std::unique_ptr<Object> ExecLiteral(AstNode* node);

};

}
}

#endif  // SETI_EXPR_EXECUTOR_H


