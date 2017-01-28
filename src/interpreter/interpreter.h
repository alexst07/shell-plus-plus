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

#ifndef SETI_INTERPRETER_H
#define SETI_INTERPRETER_H

#include <functional>

#include "symbol-table.h"
#include "ast/ast.h"
#include "objects/abstract-obj.h"

namespace seti {
namespace internal {

class RootExecutor;

class Interpreter {
 public:
  Interpreter(bool main = false);

  ~Interpreter() = default;

  inline SymbolTableStack& SymTableStack() {
    return symbol_table_stack_;
  }

  void Exec(std::string name);
  void ExecInterative(const std::function<std::string(Executor *, bool)> &func);

  ObjectPtr LookupSymbol(const std::string& name);

  Executor* ExecutorPtr();

 private:
  void RegisterVars();
  void InsertVar(const std::string& name, ObjectPtr obj);
  void RegisterFileVars(const std::string& file);

  SymbolTablePtr symbol_table_;
  SymbolTableStack symbol_table_stack_;
  std::unique_ptr<StatementList> stmt_list_;
  bool main_;
};

}
}

#endif  // SETI_INTERPRETER_H


