#include "scope-executor.h"

#include "stmt_executor.h"
#include "utils/scope-exit.h"

namespace setti {
namespace internal {

void ScopeExecutor::PushDeferStmt(Statement* stmt) {
  defer_stack_.push(stmt);
}

Executor* ScopeExecutor::GetMainExecutor() {
  if (main_exec_) {
    return this;
  } else {
    if (parent() != nullptr) {
      return parent()->GetMainExecutor();
    }

    return nullptr;
  }
}

void ScopeExecutor::ExecuteDeferStack() {
  executed_defer_ = true;

  StmtExecutor stmt_exec(this, symbol_table_stack());
  while (defer_stack_.size() > 0) {
    stmt_exec.Exec(defer_stack_.top());
    defer_stack_.pop();
  }
}

void RootExecutor::Exec(AstNode* node) {
  // scope exit case an excpetion thrown
  auto cleanup = MakeScopeExit([&]() {
    // remove the scope
    ExecuteDeferStack();
  });
  IgnoreUnused(cleanup);

  StmtListExecutor executor(this, symbol_table_stack());
  executor.Exec(node);
}

void BlockExecutor::Exec(AstNode* node) {
  Block* block_node = static_cast<Block*>(node);
  StmtListExecutor executor(this, symbol_table_stack());
  executor.Exec(block_node->stmt_list());
}

void BlockExecutor::set_stop(StopFlag flag) {
  if (parent() == nullptr) {
    return;
  }

  parent()->set_stop(flag);
}

}
}
