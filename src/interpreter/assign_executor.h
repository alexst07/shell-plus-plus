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

// Class to execute assignment operation
//
// on this class some methods return reference for shared_ptr
// it is not common in C++, but on this case, the objective
// is not increment the counter, but change the variable
class AssignExecutor: public Executor {
 public:
  AssignExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  // Entry point to execute assign operations
  void Exec(AstNode* node);

  ObjectPtr& AssignIdentifier(AstNode* node, bool create = false);

  ObjectPtr& AssignArray(AstNode* node);

  // Gets the pointer of a symbol to assign a value
  ObjectPtr& AssignmentAcceptorExpr(AstNode* node);

  std::vector<std::reference_wrapper<ObjectPtr>>
  AssignList(AstNode* node);

  // Executes assignable values, that could be a list
  // with functions or expressions
  std::unique_ptr<Object> ExecAssignable(AstNode* node);

  // Executes assignable list, it can be function or expression
  std::vector<std::unique_ptr<Object>> ExecAssignableList(AstNode* node);

  ObjectPtr& RefArray(Array& array_node, ArrayObject& obj);

  ObjectPtr& RefTuple(Array& array_node, TupleObject& obj);

  ObjectPtr& RefMap(Array& array_node, MapObject& obj);

  void set_stop(StopFlag flag) override;
};

}
}

#endif  // SETI_ASSIGN_EXECUTOR_H


