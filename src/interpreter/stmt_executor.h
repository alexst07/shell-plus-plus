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

class FuncDeclExecutor: public Executor {
 public:
  FuncDeclExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
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

class BlockExecutor: public Executor {
 public:
  // the last parameter on Executor constructor means this is NOT the
  // root executor
  BlockExecutor(SymbolTableStack& symbol_table_stack)
      : Executor(nullptr, symbol_table_stack, false) {}

  void Exec(AstNode* node) {
    Block* block_node = static_cast<Block*>(node);
    StmtListExecutor executor(this, symbol_table_stack());
    executor.Exec(block_node->stmt_list());
  }
};

}
}

#endif  // SETI_STMT_EXECUTOR_H


