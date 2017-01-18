#include <iostream>
#include <fstream>
#include <sstream>

#include "parser/parser.h"
#include "ast/ast-printer.h"
#include "interpreter/symbol-table.h"
#include "interpreter/executor.h"
#include "interpreter/scope-executor.h"
#include "objects/object-factory.h"
#include "modules/path.h"
#include "modules/std-funcs.h"

int main(int argc, char **argv) {
  using namespace seti::internal;

  SymbolTablePtr symbol_table(SymbolTablePtr(new SymbolTable));
  SymbolTableStack symbol_table_stack(symbol_table);

  AlocTypes(symbol_table_stack);
  module::stdf::RegisterModule(symbol_table_stack);
  module::path::RegisterModule(symbol_table_stack);

  RootExecutor executor(symbol_table_stack);
  bool concat = false;
  std::string str_source;

  while (true) {
    char input[100];

    if (concat) {
      std::cout << "| ";
    } else {
      std::cout << "> ";
    }

    if (!std::cin.getline(input,sizeof(input))) {
      break;
    }

    if (concat) {
      str_source += std::string("\n") +  input;
    } else {
      str_source = input;
    }

    Lexer l(str_source);
    TokenStream ts = l.Scanner();
    Parser p(std::move(ts));
    auto res = p.AstGen();
    std::unique_ptr<StatementList> stmt_list = res.MoveAstNode();

    if (p.nerrors() == 0) {
      concat = false;
      executor.Exec(stmt_list.get());
    } else {
      if (p.StmtIncomplete()) {
        concat = true;
        continue;
      } else {
        concat = false;
        std::cout << "Parser error analysis:\n";
        auto msgs = p.Msgs();
        for (const auto& msg : msgs) {
          std::cout << msg << "\n";
        }
      }
    }

  }

}
