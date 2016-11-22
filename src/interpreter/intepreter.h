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

  void set_stop(StopFlag flag) override {

  }
};

// Temporary declaration of functions
class PrintFunc: public FuncObject {
 public:
  PrintFunc(): FuncObject() {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params) {
    for (auto& e: params) {
      e->Print();
    }

    std::cout << "\n";

    return ObjectPtr(new NullObject);
  }
};

class Interpreter {
 public:
  Interpreter() {
    ObjectPtr obj(new PrintFunc);
    SymbolAttr symbol(obj, true);
    symbol_table_.InsertEntry("print", std::move(symbol));

    ObjectPtr type_null(new NullType);
    SymbolAttr symbol_null(type_null, true);
    symbol_table_.InsertEntry(static_cast<const NullType&>(*type_null).name(),
                              std::move(symbol_null));

    ObjectPtr type_int(new IntType);
    SymbolAttr symbol_int(type_int, true);
    symbol_table_.InsertEntry(static_cast<const IntType&>(*type_int).name(),
                              std::move(symbol_int));

    ObjectPtr type_real(new IntType);
    SymbolAttr symbol_real(type_real, true);
    symbol_table_.InsertEntry(static_cast<const RealType&>(*type_real).name(),
                              std::move(symbol_real));

    ObjectPtr type_str(new StringType);
    SymbolAttr symbol_str(type_str, true);
    symbol_table_.InsertEntry(static_cast<const StringType&>(*type_str).name(),
                              std::move(symbol_str));

    ObjectPtr type_array(new ArrayType);
    SymbolAttr symbol_array(type_array, true);
    symbol_table_.InsertEntry(static_cast<const ArrayType&>(*type_array).name(),
                              std::move(symbol_array));

    ObjectPtr type_tuple(new TupleType);
    SymbolAttr symbol_tuple(type_tuple, true);
    symbol_table_.InsertEntry(static_cast<const TupleType&>(*type_tuple).name(),
                              std::move(symbol_tuple));

    ObjectPtr type_map(new MapType);
    SymbolAttr symbol_map(type_map, true);
    symbol_table_.InsertEntry(static_cast<const MapType&>(*type_map).name(),
                              std::move(symbol_map));
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


