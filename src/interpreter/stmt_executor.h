#ifndef SETI_STMT_EXECUTOR_H
#define SETI_STMT_EXECUTOR_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <tuple>

#include "ast/ast.h"
#include "obj_type.h"
#include "executor.h"
#include "object-factory.h"

namespace setti {
namespace internal {

class StmtListExecutor: public Executor {
 public:
  StmtListExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack), stop_flag_(StopFlag::kGo) {}

  void Exec(AstNode* node);

  void set_stop(StopFlag flag) override;

 private:
  StopFlag stop_flag_;
};

class FuncDeclExecutor: public Executor {
 public:
  FuncDeclExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack)
      , obj_factory_(symbol_table_stack) {}

  void Exec(AstNode* node);

  void set_stop(StopFlag flag) override;

 private:
  ObjectFactory obj_factory_;
};

class StmtExecutor: public Executor {
 public:
  StmtExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  // Entry point to execute expression
  void Exec(AstNode* node);

  void set_stop(StopFlag flag) override;
};

class BlockExecutor: public Executor {
 public:
  // the last parameter on Executor constructor means this is NOT the
  // root executor
  BlockExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack, false) {}

  void Exec(AstNode* node) {
    Block* block_node = static_cast<Block*>(node);
    StmtListExecutor executor(this, symbol_table_stack());
    executor.Exec(block_node->stmt_list());
  }

  void set_stop(StopFlag flag) override {
    parent()->set_stop(flag);
  }
};

class ReturnExecutor: public Executor {
 public:
  ReturnExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack)
      , obj_factory_(symbol_table_stack) {}

  // Entry point to execute expression
  void Exec(AstNode* node);

  void set_stop(StopFlag flag) override;

 private:
  ObjectFactory obj_factory_;
};

}
}

#endif  // SETI_STMT_EXECUTOR_H


