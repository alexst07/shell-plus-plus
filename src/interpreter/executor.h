#ifndef SETI_EXECUTOR_H
#define SETI_EXECUTOR_H

#include <string>
#include <memory>
#include <unordered_map>
#include <tuple>

#include "ast/ast.h"
#include "obj_type.h"
#include "symbol_table.h"

namespace setti {
namespace internal {

class Executor {
 public:
  enum class StopFlag {
    kGo,
    kReturn,
    kBreak,
    kContinue,
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

  virtual void set_stop(StopFlag flag) {
    if (parent_ != nullptr) {
      parent_->set_stop(flag);
    }
  }

  Executor* parent() const noexcept {
    return parent_;
  }

  virtual Executor* GetMainExecutor() {
    if (parent_ != nullptr) {
      return parent_->GetMainExecutor();
    }

    return nullptr;
  }

 protected:
  SymbolTableStack& symbol_table_stack() {
    return symbol_table_stack_;
  }

  inline bool is_root() const{
    return is_root_;
  }

  // this method is used for decide if some statement
  // can be called on that moment, for example
  // break statement can be called only inside loops
  // and switch control
  virtual bool inside_loop() {
    if (parent_ != nullptr) {
      return parent_->inside_loop();
    }

    return false;
  }

  // this method is used by switch control flow
  // because some statement can be called only
  // inside a switch block
  virtual bool inside_switch() {
    if (parent_ != nullptr) {
      return parent_->inside_switch();
    }

    return false;
  }

  virtual Executor* GetBlockParent() {
    if (parent_ != nullptr) {
      return parent_->GetBlockParent();
    }

    return nullptr;
  }

 private:
  Executor* parent_;
  SymbolTableStack& symbol_table_stack_;
  bool is_root_;
};

}
}

#endif  // SETI_EXECUTOR_H
