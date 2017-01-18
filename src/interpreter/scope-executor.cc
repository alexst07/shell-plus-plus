// Copyright 2016 Alex Silva Torres
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "scope-executor.h"

#include "stmt-executor.h"
#include "utils/scope-exit.h"

namespace seti {
namespace internal {

void ScopeExecutor::PushDeferStmt(std::tuple<Statement *, SymbolTableStack> s) {
  defer_stack_.push(s);
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

  while (defer_stack_.size() > 0) {
    StmtExecutor stmt_exec(this, std::get<1>(defer_stack_.top()));
    stmt_exec.Exec(std::get<0>(defer_stack_.top()));
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
