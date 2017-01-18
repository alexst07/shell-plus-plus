#ifndef SETI_MAIN_INTERPRETER_H
#define SETI_MAIN_INTERPRETER_H

#include "interpreter/symbol-table.h"
#include "ast/ast.h"

namespace seti {

using internal::Lexer;
using internal::TokenStream;
using internal::SymbolTableStack;
using internal::SymbolTablePtr;
using internal::StatementList;

class Runner {
 public:
  Runner();

  ~Runner() = default;

  inline SymbolTableStack& SymTableStack() {
    return symbol_table_stack_;
  }

  void Exec(std::string file_name);
  void ExecInterative();

 private:
  SymbolTablePtr symbol_table_;
  SymbolTableStack symbol_table_stack_;
  std::unique_ptr<StatementList> stmt_list_;
};

}

#endif  // SETI_MAIN_INTERPRETER_H


