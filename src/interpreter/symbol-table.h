// Copyright 2016 Alex Silva Torres
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SETI_SYMBOL_TABLE_H
#define SETI_SYMBOL_TABLE_H

#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <stack>
#include <boost/format.hpp>

#include "run_time_error.h"

namespace seti {
namespace internal {

class Object;
class CmdEntry;

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
      , value_(other.value_) {}

  SymbolAttr& operator=(SymbolAttr&& other) noexcept {
    if (&other == this) {
      return *this;
    }

    global_ = other.global_;
    value_ = other.value_;

    return *this;
  }

  // The symbol can't be copied, only the value of the symbol can
  SymbolAttr(const SymbolAttr& sym) {
    global_ = sym.global_;
    value_ = sym.value_;
  }

  SymbolAttr& operator=(const SymbolAttr& sym) {
    global_ = sym.global_;
    value_ = sym.value_;

    return *this;
  }

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
  enum class TableType {
    SCOPE_TABLE,
    FUNC_TABLE,
    CLASS_TABLE
  };

  using SymbolMap = std::unordered_map<std::string, SymbolAttr>;
  using CmdMap = std::unordered_map<std::string, std::shared_ptr<CmdEntry>>;
  using SymbolIterator = SymbolMap::iterator;
  using CmdIterator = CmdMap::iterator;
  using SymbolConstIterator = SymbolMap::const_iterator;
  using CmdAliasMap = std::unordered_map<std::string, std::vector<std::string>>;

  SymbolTable(TableType type = TableType::SCOPE_TABLE): type_(type) {}

  SymbolTable(const SymbolTable& sym_table) {
    map_ = sym_table.map_;
    type_ = sym_table.type_;
  }

  static SymbolTablePtr Create(TableType type = TableType::SCOPE_TABLE) {
    return SymbolTablePtr(new SymbolTable(type));
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
      it->second.set_value(value);
      return;
    }

    // declare variable always as local
    SymbolAttr symbol(value, false);
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
    map_.insert(it, std::move(std::pair<std::string, SymbolAttr>(
        name, std::move(symbol))));

    return true;
  }

  void SetCmd(const std::string& name, std::shared_ptr<CmdEntry> cmd) {
    cmd_map_[name] = cmd;
  }

  inline std::shared_ptr<CmdEntry> LookupCmd(const std::string& name) {
    auto it = cmd_map_.find(name);

    if (it != cmd_map_.end()) {
      return it->second;
    }

    return std::shared_ptr<CmdEntry>(nullptr);
  }

  inline bool RemoveCmd(const std::string& name) {
    if (cmd_map_.erase(name) == 1) {
      return true;
    } else {
      return false;
    }
  }

  // insert alias for command, this must be global even if declared
  // inside an if or loop, and in the import must me copied
  void SetCmdAlias(const std::string& name, std::vector<std::string>&& cmd) {
    cmd_alias_[name] = std::move(cmd);
  }

  bool ExistsCmdAlias(const std::string& name) const {
    auto it = cmd_alias_.find(name);

    if (it == cmd_alias_.end()) {
      return false;
    }

    return true;
  }

  const std::vector<std::string>& GetCmdAlias(const std::string& name) {
    return cmd_alias_[name];
  }

  inline SymbolIterator Lookup(const std::string& name) {
    return map_.find(name);
  }

  inline bool Remove(const std::string& name) {
    if (map_.erase(name) == 1) {
      return true;
    } else {
      return false;
    }
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

  TableType Type() const noexcept {
    return type_;
  }

  void Clear() {
    map_.clear();
  }

 private:
  SymbolMap map_;
  CmdMap cmd_map_;
  TableType type_;
  CmdAliasMap cmd_alias_;
};

class SymbolTableStackBase {
 public:
  // Insert a table on the stack
  virtual void Push(SymbolTablePtr table, bool is_main = false) = 0;

  // Create a new table on the stack
  virtual void NewTable(bool is_main = false) = 0;

  virtual void Pop() = 0;

  // Search in all stack an return the refence for the symbol if
  // it exists, or if create = true, create a new symbol if it
  // doesn't exists and return its reference
  virtual SymbolAttr& Lookup(const std::string& name, bool create) = 0;

  virtual std::tuple<std::shared_ptr<Object>,bool> LookupObj(
      const std::string& name) = 0;

  virtual bool InsertEntry(const std::string& name, SymbolAttr&& symbol) = 0;

  virtual void SetEntry(const std::string& name,
                        std::shared_ptr<Object> value) = 0;

  virtual void SetEntryOnFunc(const std::string& name,
                              std::shared_ptr<Object> value) = 0;

  virtual SymbolTablePtr MainTable() const noexcept = 0;

  virtual void SetFirstAsMain() = 0;

  virtual void Dump() = 0;
};

class SymbolTableStack: public SymbolTableStackBase {
 public:
  SymbolTableStack(SymbolTablePtr symbol_table = SymbolTablePtr(nullptr)) {
    if (symbol_table) {
      main_table_ = symbol_table;
    }
  }

  SymbolTableStack(const SymbolTableStack& st) {
    stack_ = st.stack_;
    main_table_ = st.main_table_;
  }

  SymbolTableStack& operator=(const SymbolTableStack& st) {
    stack_ = st.stack_;
    main_table_ = st.main_table_;

    return *this;
  }

  SymbolTableStack(SymbolTableStack&& st) {
    stack_ = std::move(st.stack_);
    main_table_ = st.main_table_;
  }

  SymbolTableStack& operator=(SymbolTableStack&& st) {
    stack_ = std::move(st.stack_);
    main_table_ = st.main_table_;

    return *this;
  }

  // Insert a table on the stack
  void Push(SymbolTablePtr table, bool is_main = false) override {
    if (is_main) {
      main_table_ = table;
      return;
    }

    stack_.push_back(table);
  }

  // Create a new table on the stack
  void NewTable(bool is_main = false) override {
    SymbolTablePtr table(new SymbolTable);
    if (is_main) {
      main_table_ = table;
    }

    stack_.push_back(std::move(table));
  }

  void Pop() override {
    stack_.pop_back();
  }

  // Search in all stack an return the refence for the symbol if
  // it exists, or if create = true, create a new symbol if it
  // doesn't exists and return its reference
  SymbolAttr& Lookup(const std::string& name, bool create) override {
    for (int i = (stack_.size() - 1); i >= 0 ; i--) {
      auto it_obj = stack_.at(i)->Lookup(name);

      if (it_obj != stack_.at(i)->end()) {
        return it_obj->second;
      }
    }

    // search on main table if no symbol was found
    auto it_obj = main_table_.lock()->Lookup(name);

    if (it_obj != main_table_.lock()->end()) {
      return it_obj->second;
    }

    if (create) {
      if (stack_.size() > 0) {
        SymbolAttr& ref = stack_.back()->SetValue(name);
        return ref;
      } else {
        SymbolAttr& ref = main_table_.lock()->SetValue(name);
        return ref;
      }
    }

    throw RunTimeError(RunTimeError::ErrorCode::SYMBOL_NOT_FOUND,
                       boost::format("symbol %1% not found")% name);
  }

  std::tuple<std::shared_ptr<Object>,bool>
  LookupObj(const std::string& name) override {
    for (int i = (stack_.size() - 1); i >= 0 ; i--) {
      auto it_obj = stack_.at(i)->Lookup(name);

      if (it_obj != stack_.at(i)->end()) {
        return std::tuple<std::shared_ptr<Object>,bool>(
              it_obj->second.SharedAccess(), true);
      }
    }

    // search on main table if no symbol was found
    auto it_obj = main_table_.lock()->Lookup(name);

    if (it_obj != main_table_.lock()->end()) {
      if (it_obj->second.global()) {
      return std::tuple<std::shared_ptr<Object>,bool>(
            it_obj->second.SharedAccess(), true);
      }
    }

    return std::tuple<std::shared_ptr<Object>,bool>(
          std::shared_ptr<Object>(nullptr), false);
  }

  bool InsertEntry(const std::string& name, SymbolAttr&& symbol) override {
    if (stack_.size() > 0) {
      return stack_.back()->SetValue(name, std::move(symbol));
    }

    return main_table_.lock()->SetValue(name, std::move(symbol));
  }

  void SetEntry(const std::string& name,
                std::shared_ptr<Object> value) override {
    if (stack_.size() > 0) {
      stack_.back()->SetValue(name, value);
    }

    main_table_.lock()->SetValue(name, value);
  }

  std::shared_ptr<CmdEntry> LookupCmd(const std::string& name) {
    std::shared_ptr<CmdEntry> cmd = main_table_.lock()->LookupCmd(name);

    return cmd;
  }

  void SetCmd(const std::string& name, std::shared_ptr<CmdEntry> cmd) {
    main_table_.lock()->SetCmd(name, cmd);
  }

  void SetCmdAlias(const std::string& name, std::vector<std::string>&& cmd) {
    main_table_.lock()->SetCmdAlias(name, std::move(cmd));
  }

  bool ExistsCmdAlias(const std::string& name) const {
    return main_table_.lock()->ExistsCmdAlias(name);
  }

  const std::vector<std::string>& GetCmdAlias(const std::string& name) {
    return main_table_.lock()->GetCmdAlias(name);
  }

  // insert the object on the table stack of the function
  void SetEntryOnFunc(const std::string& name,
                      std::shared_ptr<Object> value) override {
    // search the last function table inserted
    for (int i = stack_.size() - 1; i >= 0; i--) {
      if (stack_.at(i)->Type() == SymbolTable::TableType::FUNC_TABLE) {
        stack_.at(i)->SetValue(name, value);
      }
    }
  }

  SymbolTablePtr MainTable() const noexcept override {
    return main_table_.lock();
  }

  void Append(const SymbolTableStack& stack) {
    for (auto table: stack.stack_) {
      auto sym_ptr = SymbolTablePtr(new SymbolTable(*table));
      stack_.push_back(sym_ptr);
    }
  }

  void Append(std::vector<SymbolTablePtr>&& stack) {
    for (auto table: stack) {
      auto sym_ptr = SymbolTablePtr(new SymbolTable(*table));
      stack_.push_back(sym_ptr);
    }
  }

  void SetFirstAsMain() override {
    main_table_ = *stack_.begin();
  }

  bool HasFuncTable() const noexcept {
    for (auto& table: stack_) {
      if (table->Type() == SymbolTable::TableType::FUNC_TABLE) {
        return true;
      }
    }

    return false;
  }

  bool HasClassTable() const noexcept {
    for (auto& table: stack_) {
      if (table->Type() == SymbolTable::TableType::CLASS_TABLE) {
        return true;
      }
    }

    return false;
  }

  std::vector<SymbolTablePtr> GetUntilFuncTable() const {
    std::vector<SymbolTablePtr> statck;

    for (auto& table: stack_) {
      if (table->Type() != SymbolTable::TableType::FUNC_TABLE) {
        statck.push_back(table);
        continue;
      }

      statck.push_back(table);

      break;
    }

    return statck;
  }

  std::vector<SymbolTablePtr> GetUntilClassTable() const {
    std::vector<SymbolTablePtr> statck;

    for (auto& table: stack_) {
      if (table->Type() != SymbolTable::TableType::CLASS_TABLE) {
        statck.push_back(table);
        continue;
      }

      statck.push_back(table);

      break;
    }

    return statck;
  }

  void Dump() override {
    std::cout << "main table copy: " << main_table_.use_count() << "\n";
    main_table_.lock()->Dump();
    std::cout << "Table: " << this << " Num: " << stack_.size() << "\n";
    for (auto& e: stack_) {
      std::cout << "------\n";
      e->Dump();
    }
    std::cout << "*************\n";
  }

 private:
  std::vector<SymbolTablePtr> stack_;
  std::weak_ptr<SymbolTable> main_table_;
};

}
}

#endif  // SETI_SYMBOL_TABLE_H
