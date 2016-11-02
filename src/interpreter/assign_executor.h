#ifndef SETI_ASSIGN_EXECUTOR_H
#define SETI_ASSIGN_EXECUTOR_H

#include <string>
#include <memory>
#include <vector>
#include <tuple>

#include "ast/ast.h"
#include "ast/obj_type.h"
#include "executor.h"
#include "ast/symbol_table.h"

namespace setti {
namespace internal {

class AssignExecutor: public Executor {
 public:
  AssignExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  // Entry point to execute assign operations
  void Exec(AstNode* node);

  SymbolAttr& AssignIdentifier(AstNode* node, bool create = false);

  std::unique_ptr<Object>& AssignArray(AstNode* node);

  // Gets the pointer of a symbol to assign a value
  EntryPointer& LeftVar(AstNode* node);

  std::vector<std::reference_wrapper<EntryPointer>>
  AssignList(AstNode* node);

  // Executes assignable values, that could be a list
  // with functions or expressions
  std::unique_ptr<Object> ExecAssignable(AstNode* node);

  // Executes assignable list, it can be function or expression
  std::vector<std::unique_ptr<Object>> ExecAssignableList(AstNode* node);

  std::unique_ptr<Object>& ObjectArray(Array& array_node, ArrayObject& obj);
};

}
}

#endif  // SETI_ASSIGN_EXECUTOR_H


