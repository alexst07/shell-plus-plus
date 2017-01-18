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
