#ifndef SETI_SYMBOL_TABLE_H
#define SETI_SYMBOL_TABLE_H

#include <string>
#include <memory>
#include <unordered_map>
#include <boost/format.hpp>

#include "obj_type.h"
#include "run_time_error.h"
#include "obj_type.h"

namespace setti {
namespace internal {

class SymbolAttr: public EntryPointer {
 public:
  SymbolAttr(std::unique_ptr<Object> value, bool global)
      : EntryPointer(EntryPointer::EntryType::SYMBOL)
      , value_(std::move(value))
      , global_(global) {}

  SymbolAttr()
      : EntryPointer(EntryPointer::EntryType::SYMBOL)
      , value_(std::unique_ptr<Object>(nullptr))
      , global_(false) {}

  ~SymbolAttr() {}

  inline Object* value() const noexcept {
    return value_.get();
  }

  inline std::unique_ptr<Object>& RefValue() {
    return value_;
  }

  SymbolAttr(SymbolAttr&& other)
      : EntryPointer(EntryPointer::EntryType::SYMBOL)
      , global_(other.global_)
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


  inline void set_value(std::unique_ptr<Object> value) noexcept {
    value_ = std::move(value);
  }

  inline bool global() const noexcept {
    return global_;
  }

  void Print() const {
    value_->Print();
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

  void SetValue(const std::string& name, std::unique_ptr<Object> value) {
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

  void Dump() const {
    for (const auto& m: map_) {
      std::cout << "name: " << m.first << " ";
      m.second.Print();
      std::cout << "\n";
    }
  }

 private:
  SymbolMap map_;
};

class SymbolTableStack {
 public:
  SymbolTableStack() {
    // Table stack is creaeted with at leas one table symbol
    SymbolTable table;
    stack_.push_back(std::move(table));
  }

  // Insert a table on the stack
  inline void Push(SymbolTable&& table) {
    stack_.push_back(std::move(table));
  }

  // Create a new table on the stack
  inline void NewTable() {
    SymbolTable table;
    stack_.push_back(std::move(table));
  }

  inline void Pop() {
    stack_.pop_back();
  }

  // Search in all stack an return the refence for the symbol if
  // it exists, or if create = true, create a new symbol if it
  // doesn't exists and return its reference
  SymbolAttr& Lookup(const std::string& name, bool create) {
    auto it_obj = stack_.back().Lookup(name);

    if (it_obj != stack_.back().end()) {
      return it_obj->second;
    }

    if (stack_.size() > 1) {
      for (size_t i = (stack_.size() - 2); i >= 0 ; i++) {
        auto it_obj = stack_.at(i).Lookup(name);

        if (it_obj != stack_.at(i).end()) {
          if (!it_obj->second.global()) {
            return it_obj->second;
          }
        }
      }
    }

    if (create) {
      SymbolAttr& ref = stack_.back().SetValue(name);
      return ref;
    }

    throw RunTimeError(RunTimeError::ErrorCode::SYMBOL_NOT_FOUND,
                       boost::format("symbol: %1% not found")% name);
  }

  bool InsertEntry(const std::string& name, SymbolAttr&& symbol) {
    // the stack has always at least one symbol table
    return stack_.back().SetValue(name, std::move(symbol));
  }

  void SetEntry(const std::string& name, std::unique_ptr<Object> value) {
    // the stack has always at least one symbol table
    stack_.back().SetValue(name, std::move(value));
  }

  void Dump() {
    for (auto& table: stack_) {
      table.Dump();
    }
  }

 private:
  std::vector<SymbolTable> stack_;
};

}
}

#endif  // SETI_SYMBOL_TABLE_H

