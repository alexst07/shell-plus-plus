#ifndef SETI_EXECUTOR_H
#define SETI_EXECUTOR_H

#include <string>
#include <memory>
#include <unordered_map>
#include <tuple>

#include "ast/ast.h"
#include "ast/obj_type.h"
#include "ast/symbol_table.h"

namespace setti {
namespace internal {

class Executor {
 public:
  Executor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : parent_(parent)
      , symbol_table_stack_(symbol_table_stack){}

 protected:
  Executor* parent() const noexcept {
    return parent_;
  }

  SymbolTableStack& symbol_table_stack() {
    return symbol_table_stack_;
  }

 private:
  Executor* parent_;
  SymbolTableStack& symbol_table_stack_;
};

}
}

#endif  // SETI_EXECUTOR_H


