#ifndef SETI_SYMBOL_TABLE_H
#define SETI_SYMBOL_TABLE_H

#include <string>
#include <memory>
#include <unordered_map>
#include <boost/format.hpp>

#include "obj_type.h"
#include "run_time_error.h"

namespace setti {
namespace internal {

class Object;

class SymbolAttr {
 public:
  SymbolAttr(std::unique_ptr<Object> value, bool global)
      : value_(std::move(value))
      , global_(global) {}

  inline Object* value() const noexcept {
    return value_.get();
  }

  SymbolAttr(SymbolAttr&& other)
      : global_(other.global_), value_(std::move(other.value_)) {}

  SymbolAttr& operator=(SymbolAttr&& other) noexcept {
    if (&other == this) {
      return *this;
    }

    global_ = other.global_;
    value_ = std::move(other.value_);

    return *this;
  }

  // The symbol can't be copied, only the value of the symbol can
  SymbolAttr(const SymbolAttr&) = delete;
  SymbolAttr& operator=(const SymbolAttr&) = delete;


  inline void set_value(std::unique_ptr<Object> value) noexcept {
    value_.reset();
    value_ = std::move(value);
  }

  inline bool global() const noexcept {
    return global_;
  }

 private:
  bool global_;
  std::unique_ptr<Object> value_;
};

class SymbolTable {
 public:
  using SymbolMap = std::unordered_map<std::string, SymbolAttr>;
  using SymbolIterator = SymbolMap::iterator;
  using SymbolConstIterator = SymbolMap::const_iterator;

  SymbolTable() {}

  inline void SetValue(const std::string& name, SymbolAttr&& symbol) {
    // if the key exists only change the value
    auto it = map_.find(name);
    if (it != map_.end()) {
      it->second = std::move(symbol);
      return;
    }

    // if the key not exists create a new
    // max efficiency inserting assign begin to i
    it = map_.begin();
    map_.insert (it, std::pair<std::string, SymbolAttr&&>(
        name, std::move(symbol)));
  }

  inline SymbolIterator Lookup(const std::string& name) {
    return map_.find(name);
  }

  inline bool Remove(const std::string& name) {
    map_.erase(name);
  }

  inline SymbolConstIterator end() const noexcept {
    return map_.end();
  }

  inline SymbolConstIterator begin() const noexcept {
    return map_.begin();
  }

 private:
  SymbolMap map_;
};

class SymbolTableStack {
 public:
  SymbolTableStack();

  inline void Push(SymbolTable&& table) {
    stack_.push_back(std::move(table));
  }

  inline void NewStack() {
    SymbolTable table;
    stack_.push_back(std::move(table));
  }

  inline void Pop() {
    stack_.pop_back();
  }

  SymbolTable::SymbolIterator Lookup(const std::string& name) {
    auto it = stack_.back();
    auto it_obj = it.Lookup(name);

    if (it_obj != it.end()) {
      return it_obj;
    }

    for (size_t i = (stack_.size() - 1); i >= 0 ; i++) {
      auto it = stack_.at(i);
      auto it_obj = it.Lookup(name);

      if (it_obj != it.end()) {
        if (!it_obj->second.global()) {
          return it_obj;
        } else {
          throw RunTimeError(RunTimeError::ErrorCode::SYMBOL_NOT_FOUND,
                             boost::format("access denied for local symbol: "
                                           "%1%")% name);
        }
      }
    }

    throw RunTimeError(RunTimeError::ErrorCode::SYMBOL_NOT_FOUND,
                       boost::format("symbol: %1% not found")% name);
  }

  void InsertEntry(const std::string& name, SymbolAttr&& symbol) {
    // the stack has always at least one symbol table
    auto it = stack_.back();
    it.SetValue(name, std::move(symbol));
  }

 private:
  std::vector<SymbolTable> stack_;
};

}
}

#endif  // SETI_SYMBOL_TABLE_H

