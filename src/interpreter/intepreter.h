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
#include "object-factory.h"

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

  void set_stop(StopFlag flag) override {

  }
};

// Temporary declaration of functions
class PrintFunc: public FuncObject {
 public:
  PrintFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params) {
    for (auto& e: params) {
      e->Print();
    }

    std::cout << "\n";

    return obj_factory_.NewNull();
  }

 private:
  ObjectFactory obj_factory_;
};

class Interpreter {
 public:
  Interpreter(): obj_factory(symbol_table_) {
    AlocTypes(symbol_table_);

    SymbolTableStack sym_stack(false);
    sym_stack.Push(symbol_table_.MainTable());
    auto func_type = symbol_table_.Lookup("func", false).SharedAccess();
    ObjectPtr obj(new PrintFunc(func_type, std::move(sym_stack)));
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
  }

 private:
  SymbolTableStack symbol_table_;
  ObjectFactory obj_factory;
};

}
}

#endif  // SETI_INTERPRETER_H


