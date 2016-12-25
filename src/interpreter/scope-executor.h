#ifndef SETI_SCOPE_EXECUTOR_H
#define SETI_SCOPE_EXECUTOR_H

#include <string>
#include <memory>
#include <unordered_map>
#include <tuple>

#include "ast/ast.h"
#include "executor.h"

namespace setti {
namespace internal {

class ScopeExecutor: public Executor {
 public:
  ScopeExecutor(Executor* parent, SymbolTableStack& symbol_table_stack,
                bool is_root, bool main_exec)
      : Executor(parent, symbol_table_stack, is_root)
      , main_exec_(main_exec)
      , executed_defer_(false) {}

  void PushDeferStmt(std::tuple<Statement*, SymbolTableStack> s);

  void ExecuteDeferStack();

  ~ScopeExecutor() = default;

 protected:
  Executor* GetMainExecutor() override;

  Executor* GetBlockParent() override {
    return this;
  }

 private:
  bool main_exec_;
  bool executed_defer_;
  std::stack<std::tuple<Statement*, SymbolTableStack>> defer_stack_;
};

class RootExecutor: public ScopeExecutor {
 public:
  // the last parameter on Executor constructor means this is the
  // root executor
  RootExecutor(SymbolTableStack& symbol_table_stack)
      : ScopeExecutor(nullptr, symbol_table_stack, true, true) {}

  void Exec(AstNode* node);

  void set_stop(StopFlag flag) override {}

 protected:
  Executor* GetMainExecutor() override {
    return this;
  }
};

class BlockExecutor: public ScopeExecutor {
 public:
  // the last parameter on Executor constructor means this is NOT the
  // root executor
  BlockExecutor(Executor* parent, SymbolTableStack& symbol_table_stack,
                bool main_exec = false)
      : ScopeExecutor(parent, symbol_table_stack, false, main_exec) {}

  void Exec(AstNode* node);

  void set_stop(StopFlag flag) override;
};

}
}

#endif  // SETI_SCOPE_EXECUTOR_H
