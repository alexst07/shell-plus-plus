#ifndef SETI_INTERPRETER_H
#define SETI_INTERPRETER_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <tuple>

#include "stmt_executor.h"
#include "executor.h"
#include "parser/parser.h"
#include "parser/lexer.h"

namespace setti {
namespace internal {

class RootExecutor: public Executor {
 public:
  // the last parameter on Executor constructor means this is the
  // root executor
  RootExecutor(SymbolTableStack& symbol_table_stack)
      : Executor(nullptr, symbol_table_stack, true) {}

  void Exec(AstNode* node) {
    StmtListExecutor executor(this, symbol_table_stack());
    executor.Exec(node);
  }
};

// Temporary declaration of functions
class PrintFunc: public FuncObject {
 public:
  PrintFunc(): FuncObject() {}

  ObjectPtr Call(std::vector<ObjectPtr>&& params) {
    std::cout << "Print:\n";

    for (auto& e: params) {
      std::cout << ">> ";
      e->Print();
      std::cout << "\n";
    }

    return ObjectPtr(new NullObject);
  }
};

class Interpreter {
 public:
  Interpreter() {
    ObjectPtr obj(new PrintFunc);
    SymbolAttr symbol(obj, true);
    symbol_table_.InsertEntry("print", std::move(symbol));
  }

  void Exec(std::string name) {
    std::ifstream file(name);
    std::stringstream buffer;
    buffer << file.rdbuf();

    Lexer l(buffer.str());
    TokenStream ts = l.Scanner();
    Parser p(std::move(ts));
    auto res = p.AstGen();

    if (p.nerrors() == 0) {
      RootExecutor executor(symbol_table_);
      executor.Exec(res.NodePtr());
    } else {
      std::cout << "Parser error analysis:\n";
      auto msgs = p.Msgs();
      for (const auto& msg : msgs) {
        std::cout << msg << "\n";
      }
    }

    symbol_table_.Dump();
  }

 private:
  SymbolTableStack symbol_table_;
};

}
}

#endif  // SETI_INTERPRETER_H


