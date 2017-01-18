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

#include "runner.h"

#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <readline/readline.h>

#include "interpreter/stmt-executor.h"
#include "interpreter/executor.h"
#include "interpreter/scope-executor.h"
#include "parser/parser.h"
#include "parser/lexer.h"
#include "modules/std-funcs.h"
#include "modules/path.h"

namespace seti {

using internal::RootExecutor;
using internal::Parser;
using internal::SymbolTable;

Runner::Runner()
    : symbol_table_(SymbolTablePtr(new SymbolTable))
    , symbol_table_stack_(symbol_table_) {
  AlocTypes(symbol_table_stack_);

  internal::module::stdf::RegisterModule(symbol_table_stack_);
  internal::module::path::RegisterModule(symbol_table_stack_);
}

void Runner::Exec(std::string name) {
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
    try {
      executor.Exec(stmt_list_.get());
    } catch (RunTimeError& e) {
      std::cout << "Error: " << e.pos().line << ": " << e.pos().col
                << ": " << e.what() << "\n";
    }
  } else {
    auto msgs = p.Msgs();
    for (const auto& msg : msgs) {
      std::cout << msg << "\n";
    }
  }
}

void Runner::ExecInterative() {
  RootExecutor executor(symbol_table_stack_);
  bool concat = false;
  std::string str_source;

  while (true) {
    char *input;

    if (concat) {
      input = readline("| ");

      if (input == nullptr) {
        break;
      }

      str_source += std::string("\n") +  input;
    } else {
      input = readline("> ");

      if (input == nullptr) {
        break;
      }

      str_source = input;
    }

    free(input);

    if (str_source.empty()) {
      continue;
    }

    Lexer l(str_source);
    TokenStream ts = l.Scanner();
    Parser p(std::move(ts));
    auto res = p.AstGen();
    std::unique_ptr<StatementList> stmt_list = res.MoveAstNode();

    if (p.nerrors() == 0) {
      concat = false;
      try {
        executor.Exec(stmt_list.get());
      } catch (RunTimeError& e) {
        std::cout << "Error: " << e.pos().line << ": " << e.pos().col
                  << ": " << e.what() << "\n";
      }
    } else {
      if (p.StmtIncomplete()) {
        concat = true;
        continue;
      } else {
        concat = false;
        auto msgs = p.Msgs();
        for (const auto& msg : msgs) {
          std::cout << msg << "\n";
        }
      }
    }
  }
}

}
