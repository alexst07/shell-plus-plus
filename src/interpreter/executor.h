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
  enum class StopFlag {
    kGo,
    kReturn,
    kBreak,
    kThrow
  };

  Executor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : parent_(parent)
      , symbol_table_stack_(symbol_table_stack)
      , is_root_(false) {}

  Executor(Executor* parent, SymbolTableStack& symbol_table_stack, bool is_root)
      : parent_(parent)
      , symbol_table_stack_(symbol_table_stack)
      , is_root_(is_root) {}

  virtual void set_stop(StopFlag flag) = 0;

  Executor* parent() const noexcept {
    return parent_;
  }

 protected:
  SymbolTableStack& symbol_table_stack() {
    return symbol_table_stack_;
  }

  inline bool is_root() const{
    return is_root_;
  }

 private:
  Executor* parent_;
  SymbolTableStack& symbol_table_stack_;
  bool is_root_;
};

}
}

#endif  // SETI_EXECUTOR_H


