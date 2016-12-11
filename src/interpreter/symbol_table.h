#ifndef SETI_SYMBOL_TABLE_H
#define SETI_SYMBOL_TABLE_H

#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <boost/format.hpp>

#include "run_time_error.h"

namespace setti {
namespace internal {

class Object;

class SymbolAttr {
 public:
  SymbolAttr(std::shared_ptr<Object> value, bool global)
      : value_(value)
      , global_(global) {}

  SymbolAttr()
      : value_(std::shared_ptr<Object>(nullptr))
      , global_(false) {}

  ~SymbolAttr() {}

  inline Object* value() const noexcept {
    return value_.get();
  }

  SymbolAttr(SymbolAttr&& other)
      : global_(other.global_)
      , value_(std::move(other.value_)) {}

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

  std::shared_ptr<Object>& Ref() noexcept {
    return value_;
  }

  std::shared_ptr<Object> SharedAccess() const noexcept {
    return value_;
  }

  inline void set_value(std::shared_ptr<Object> value) noexcept {
    value_ = value;
  }

  inline bool global() const noexcept {
    return global_;
  }

 private:
  bool global_;
  std::shared_ptr<Object> value_;
};

class SymbolTable;
typedef std::shared_ptr<SymbolTable> SymbolTablePtr;

class SymbolTable {
 public:
  using SymbolMap = std::unordered_map<std::string, SymbolAttr>;
  using SymbolIterator = SymbolMap::iterator;
  using SymbolConstIterator = SymbolMap::const_iterator;

  SymbolTable(bool is_func = false): is_func_(is_func) {}

  static SymbolTablePtr Create(bool is_func = false) {
    return SymbolTablePtr(new SymbolTable(is_func));
  }

  // Return a reference for symbol if it exists or create a new
  // and return the reference
  SymbolAttr& SetValue(const std::string& name) {
    auto it = map_.find(name);
    if (it != map_.end()) {
      return it->second;
    } else {
      it = map_.begin();

      // declare a variable as local
      SymbolAttr symbol;
      SymbolIterator it_symbol = map_.insert (it, std::move(
          std::pair<std::string, SymbolAttr>(name, std::move(symbol))));
      return it_symbol->second;
    }
  }

  void SetValue(const std::string& name, std::shared_ptr<Object> value) {
    auto it = map_.find(name);
    if (it != map_.end()) {
      it->second.set_value(std::move(value));
      return;
    }

    // declare variable always as local
    SymbolAttr symbol(std::move(value), false);
    it = map_.begin();
    map_.insert (it, std::move(std::pair<std::string, SymbolAttr>(
        name, std::move(symbol))));
  }

  bool SetValue(const std::string& name, SymbolAttr&& symbol) {
    // if the key exists only change the value
    auto it = map_.find(name);
    if (it != map_.end()) {
      return false;
    }

    // if the key not exists create a new
    // max efficiency inserting assign begin to i
    it = map_.begin();
    map_.insert (it, std::move(std::pair<std::string, SymbolAttr>(
        name, std::move(symbol))));
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

  void Dump() {
    for (const auto& e: map_) {
      std::cout << e.first << "\n";
    }
  }

  bool IsFunc() const noexcept {
    return is_func_;
  }

 private:
  SymbolMap map_;
  bool is_func_;
};

class SymbolTableStack {
 public:
  SymbolTableStack(bool no_table = false) {
    if (no_table) {
      return;
    }

    // Table stack is creaeted with at least one table symbol
    SymbolTablePtr table(new SymbolTable);
    main_table_ = table;
    stack_.push_back(std::move(table));
  }

  SymbolTableStack(const SymbolTableStack& st) {
    stack_ = st.stack_;
    main_table_ = st.main_table_;
  }

  // Insert a table on the stack
  inline void Push(SymbolTablePtr table, bool is_main = false) {
    if (is_main) {
      main_table_ = table;
    }

    stack_.push_back(table);
  }

  // Create a new table on the stack
  inline void NewTable(bool is_main = false) {
    SymbolTablePtr table(new SymbolTable);
    if (is_main) {
      main_table_ = table;
    }

    stack_.push_back(std::move(table));
  }

  inline void Pop() {
    stack_.pop_back();
  }

  // Search in all stack an return the refence for the symbol if
  // it exists, or if create = true, create a new symbol if it
  // doesn't exists and return its reference
  SymbolAttr& Lookup(const std::string& name, bool create) {
    auto it_obj = stack_.back()->Lookup(name);

    if (it_obj != stack_.back()->end()) {
      return it_obj->second;
    }

    if (stack_.size() > 1) {
      for (int i = (stack_.size() - 2); i >= 0 ; i--) {
        auto it_obj = stack_.at(i)->Lookup(name);

        if (it_obj != stack_.at(i)->end()) {
          return it_obj->second;
        }
      }
    }

    if (create) {
      SymbolAttr& ref = stack_.back()->SetValue(name);
      return ref;
    }

    throw RunTimeError(RunTimeError::ErrorCode::SYMBOL_NOT_FOUND,
                       boost::format("symbol %1% not found")% name);
  }

  std::tuple<std::shared_ptr<Object>,bool> LookupObj(const std::string& name) {
    auto it_obj = stack_.back()->Lookup(name);

    if (it_obj != stack_.back()->end()) {
      return std::tuple<std::shared_ptr<Object>,bool>(
            it_obj->second.SharedAccess(), true);
    }

    if (stack_.size() > 1) {
      for (int i = (stack_.size() - 2); i >= 0 ; i--) {
        auto it_obj = stack_.at(i)->Lookup(name);

        if (it_obj != stack_.at(i)->end()) {
          if (it_obj->second.global()) {
            return std::tuple<std::shared_ptr<Object>,bool>(
                  it_obj->second.SharedAccess(), true);
          }
        }
      }
    }

    return std::tuple<std::shared_ptr<Object>,bool>(
          std::shared_ptr<Object>(nullptr), false);
  }

  bool InsertEntry(const std::string& name, SymbolAttr&& symbol) {
    // the stack has always at least one symbol table
    return stack_.back()->SetValue(name, std::move(symbol));
  }

  void SetEntry(const std::string& name, std::shared_ptr<Object> value) {
    // the stack has always at least one symbol table
    stack_.back()->SetValue(name, std::move(value));
  }

  void SetEntryOnFunc(const std::string& name, std::shared_ptr<Object> value) {
    // search the last function table inserted
    for (int i = stack_.size() - 1; i >= 0; i--) {
      if (stack_.at(i)->IsFunc()) {
        stack_.at(i)->SetValue(name, std::move(value));
      }
    }
  }

  SymbolTablePtr MainTable() const noexcept {
    return main_table_;
  }

  void Append(const SymbolTableStack& stack) {
    for (auto table: stack.stack_) {
      stack_.push_back(table);
    }
  }

  void SetFirstAsMain() {
    main_table_ = *stack_.begin();
  }

  void Dump() {
    std::cout << "Table: " << this << " Num: " << stack_.size() << "\n";
    for (auto& e: stack_) {
      std::cout << "------\n";
      e->Dump();
    }
  }

 private:
  std::vector<SymbolTablePtr> stack_;
  SymbolTablePtr main_table_;
};

}
}

#endif  // SETI_SYMBOL_TABLE_H
