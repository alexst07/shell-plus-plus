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

namespace setti {

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
