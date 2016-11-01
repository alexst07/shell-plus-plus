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

template<class Derived>
class LeftPointer {
 public:
  enum class EntryType: uint8_t {
    SYMBOL,
    OBJECT
  };

  LeftPointer(Derived& derived): derived_(derived) {}

  EntryType entry_type() const noexcept {
    return derived_.type();
  }

 protected:
  Derived& derived_;
};

class SymbolAttr: public LeftPointer<SymbolAttr> {
 public:
  SymbolAttr(std::unique_ptr<Object> value, bool global)
      : LeftPointer(*this)
      , value_(std::move(value))
      , global_(global) {}

  LeftPointer::EntryType entry_type() const noexcept {
    return LeftPointer::EntryType::SYMBOL;
  }

  inline Object* value() const noexcept {
    return value_.get();
  }

  SymbolAttr(SymbolAttr&& other)
      : LeftPointer(*this)
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

  SymbolAttr& Lookup(const std::string& name) {
    auto it_obj = stack_.back().Lookup(name);

    if (it_obj != stack_.back().end()) {
      return it_obj->second;
    }

    for (size_t i = (stack_.size() - 1); i >= 0 ; i++) {
      auto it_obj = stack_.at(i).Lookup(name);

      if (it_obj != stack_.at(i).end()) {
        if (!it_obj->second.global()) {
          return it_obj->second;
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

  bool InsertEntry(const std::string& name, SymbolAttr&& symbol) {
    // the stack has always at least one symbol table
    return stack_.back().SetValue(name, std::move(symbol));
  }

  void SetEntry(const std::string& name, std::unique_ptr<Object> value) {
    // the stack has always at least one symbol table
    stack_.back().SetValue(name, std::move(value));
  }

 private:
  std::vector<SymbolTable> stack_;
};

}
}

#endif  // SETI_SYMBOL_TABLE_H

