#ifndef SETI_STMT_EXECUTOR_H
#define SETI_STMT_EXECUTOR_H

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

class StmtListExecutor: public Executor {
 public:
  StmtListExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  void Exec(AstNode* node);
};

class StmtExecutor: public Executor {
 public:
  StmtExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  // Entry point to execute expression
  void Exec(AstNode* node);
};

}
}

#endif  // SETI_STMT_EXECUTOR_H


