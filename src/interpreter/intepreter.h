#ifndef SETI_INTERPRETER_H
#define SETI_INTERPRETER_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <tuple>

#include "stmt_executor.h"
#include "executor.h"
#include "scope-executor.h"
#include "parser/parser.h"
#include "parser/lexer.h"
#include "object-factory.h"

namespace setti {
namespace internal {

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
  Interpreter()
      : symbol_table_(SymbolTablePtr(new SymbolTable))
      , symbol_table_stack_(symbol_table_)
      , obj_factory(symbol_table_stack_) {
    AlocTypes(symbol_table_stack_);

    symbol_table_stack_.Dump();

    SymbolTableStack sym_stack;
    sym_stack.Push(symbol_table_stack_.MainTable());
    auto func_type = symbol_table_stack_.Lookup("func", false).SharedAccess();
    ObjectPtr obj(new PrintFunc(func_type, std::move(sym_stack)));
    SymbolAttr symbol(obj, true);
    symbol_table_stack_.InsertEntry("print", std::move(symbol));
  }

  ~Interpreter() {
    // avoid errors of memory leak using sanytise flag
    symbol_table_.~shared_ptr();
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
      RootExecutor executor(symbol_table_stack_);
      executor.Exec(res.NodePtr());
    } else {
      std::cout << "Parser error analysis:\n";
      auto msgs = p.Msgs();
      for (const auto& msg : msgs) {
        std::cout << msg << "\n";
      }
    }

//    symbol_table_->Clear();
  }

 private:
  SymbolTablePtr symbol_table_;
  SymbolTableStack symbol_table_stack_;
  ObjectFactory obj_factory;
};

}
}

#endif  // SETI_INTERPRETER_H


