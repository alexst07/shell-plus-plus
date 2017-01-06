#ifndef SETI_INTERPRETER_H
#define SETI_INTERPRETER_H

#include "symbol_table.h"
#include "ast/ast.h"

namespace setti {
namespace internal {

class Interpreter {
 public:
  Interpreter();

  ~Interpreter();

  inline SymbolTableStack& SymTableStack() {
    return symbol_table_stack_;
  }

  void Exec(std::string name);

 private:
  SymbolTablePtr symbol_table_;
  SymbolTableStack symbol_table_stack_;
  std::unique_ptr<StatementList> stmt_list_;
};

}
}

#endif  // SETI_INTERPRETER_H


