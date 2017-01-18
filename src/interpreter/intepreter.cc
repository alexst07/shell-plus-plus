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

#include "intepreter.h"

#include <string>
#include <memory>
#include <iostream>
#include <fstream>

#include "stmt-executor.h"
#include "executor.h"
#include "scope-executor.h"
#include "parser/parser.h"
#include "parser/lexer.h"
#include "modules/std-funcs.h"
#include "modules/path.h"

namespace seti {
namespace internal {

Interpreter::Interpreter()
    : symbol_table_(SymbolTablePtr(new SymbolTable))
    , symbol_table_stack_(symbol_table_) {
  AlocTypes(symbol_table_stack_);

  module::stdf::RegisterModule(symbol_table_stack_);
  module::path::RegisterModule(symbol_table_stack_);
}

Interpreter::~Interpreter() {
  // avoid errors of memory leak using sanytise flag
  symbol_table_.~shared_ptr();
}

void Interpreter::Exec(std::string name) {
  std::ifstream file(name);
  std::stringstream buffer;
  buffer << file.rdbuf();

  Lexer l(buffer.str());
  TokenStream ts = l.Scanner();
  Parser p(std::move(ts));
  auto res = p.AstGen();
  stmt_list_ = res.MoveAstNode();

  if (p.nerrors() == 0) {
    RootExecutor executor(symbol_table_stack_);
    executor.Exec(stmt_list_.get());
  } else {
    std::cout << "Parser error analysis:\n";
    auto msgs = p.Msgs();
    for (const auto& msg : msgs) {
      std::cout << msg << "\n";
    }
  }

//  symbol_table_->Clear();
}

}
}
