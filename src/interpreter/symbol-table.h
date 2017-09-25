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

#ifndef SHPP_SYMBOL_TABLE_H
#define SHPP_SYMBOL_TABLE_H

#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <stack>
#include <boost/format.hpp>

#include "run_time_error.h"

namespace shpp {
namespace internal {

class Object;
class CmdEntry;
class SymbolTableStack;
class SysSymbolTable;

class SymbolAttr {
 public:
  SymbolAttr(std::shared_ptr<Object> value, bool global)
      : value_(value)
      , global_(global) {}

  SymbolAttr(bool global = false)
      : value_(std::shared_ptr<Object>(nullptr))
      , global_(global) {}

  ~SymbolAttr() {}

  inline Object* value() const noexcept {
    return value_.get();
  }

  SymbolAttr(SymbolAttr&& other)
      : value_(other.value_)
      , global_(other.global_) {}

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

  inline std::shared_ptr<Object>& Ref() noexcept {
    return value_;
  }

  inline std::shared_ptr<Object> SharedAccess() const noexcept {
    return value_;
  }

  inline void set_value(std::shared_ptr<Object> value) noexcept {
    value_ = value;
  }

  inline bool global() const noexcept {
    return global_;
  }

 private:
  std::shared_ptr<Object> value_;
  bool global_;
};

class SymbolTable;
typedef std::shared_ptr<SymbolTable> SymbolTablePtr;

class SymbolTable {
 public:
  enum class TableType {
    SYS_TABLE,
    SCOPE_TABLE,
    FUNC_TABLE,
    LAMBDA_TABLE,
    CLASS_TABLE
  };

  using SymbolMap = std::unordered_map<std::string, SymbolAttr>;
  using SymbolIterator = SymbolMap::iterator;
  using SymbolConstIterator = SymbolMap::const_iterator;

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
  SymbolAttr& SetValue(const std::string& name, bool global = false);

  void SetValue(const std::string& name, std::shared_ptr<Object> value,
      bool global = false);

  bool SetValue(const std::string& name, SymbolAttr&& symbol);

  inline SymbolIterator Lookup(const std::string& name) {
    return map_.find(name);
  }

  SymbolAttr& Lookup(const std::string& name, bool create);

  inline bool Remove(const std::string& name) {
    if (map_.erase(name) == 1) {
      return true;
    }

    return false;
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

  inline TableType Type() const noexcept {
    return type_;
  }

  void Clear() {
    map_.clear();
  }

  std::map<std::string, std::shared_ptr<Object>> SymMap();

 private:
  SymbolMap map_;
  TableType type_;
};

void AlocTypes(SymbolTableStack& symbol_table);

class SysSymbolTable {
 public:
  using SymbolMap = std::unordered_map<std::string, SymbolAttr>;
  using SymbolIterator = SymbolMap::iterator;
  using SymbolConstIterator = SymbolMap::const_iterator;
  using CmdMap = std::unordered_map<std::string, std::shared_ptr<CmdEntry>>;
  using CmdIterator = CmdMap::iterator;
  using CmdAliasMap = std::unordered_map<std::string,
      std::vector<std::string>>;

  static SysSymbolTable *instance() {
    if (!instance_) {
      instance_ = new SysSymbolTable;
    }

    return instance_;
  }

  inline SymbolAttr& SetValue(const std::string& name, bool global = false) {
    return symbol_table_->SetValue(name, global);
  }

  inline void SetValue(const std::string& name, std::shared_ptr<Object> value,
      bool global = false) {
    symbol_table_->SetValue(name, value, global);
  }

  inline bool SetValue(const std::string& name, SymbolAttr&& symbol) {
    return symbol_table_->SetValue(name, std::move(symbol));
  }

  inline void SetCmd(const std::string& name, std::shared_ptr<CmdEntry> cmd) {
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
  inline void SetCmdAlias(const std::string& name,
      std::vector<std::string>&& cmd) {
    cmd_alias_[name] = std::move(cmd);
  }

  inline bool ExistsCmdAlias(const std::string& name) const {
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
    return symbol_table_->Lookup(name);
  }

  inline bool Remove(const std::string& name) {
    return symbol_table_->Remove(name);
  }

  inline SymbolConstIterator end() const noexcept {
    return symbol_table_->end();
  }

  inline SymbolConstIterator begin() const noexcept {
    return symbol_table_->begin();
  }

  inline void Dump() {
    symbol_table_->Dump();
  }

  inline SymbolTable::TableType Type() const noexcept {
    return symbol_table_->Type();
  }

  inline void Clear() {
    symbol_table_->Clear();
  }

  inline SymbolTablePtr ptr() {
    return symbol_table_;
  }

  friend void AlocTypes(SymbolTableStack& symbol_table) ;

 private:
  SysSymbolTable()
      : symbol_table_(SymbolTablePtr(new SymbolTable(
            SymbolTable::TableType::SYS_TABLE)))
      , type_allocated_(false) {}

  static SysSymbolTable *instance_;
  SymbolTablePtr symbol_table_;
  bool type_allocated_;
  CmdMap cmd_map_;
  CmdAliasMap cmd_alias_;
};

class SymbolTableStack {
 public:
  SymbolTableStack(SymbolTablePtr symbol_table = SymbolTablePtr(nullptr));

  SymbolTableStack(const SymbolTableStack& st) {
    stack_ = st.stack_;
    main_table_ = st.main_table_;
    class_table_ = st.class_table_;
    sys_table_ = st.sys_table_;
    pos_func_table_ = st.pos_func_table_;
    pos_class_table_ = st.pos_class_table_;
    pos_lambda_table_ = st.pos_lambda_table_;
  }

  SymbolTableStack& operator=(const SymbolTableStack& st) {
    stack_ = st.stack_;
    main_table_ = st.main_table_;
    class_table_ = st.class_table_;
    sys_table_ = st.sys_table_;
    pos_func_table_ = st.pos_func_table_;
    pos_class_table_ = st.pos_class_table_;
    pos_lambda_table_ = st.pos_lambda_table_;

    return *this;
  }

  SymbolTableStack(SymbolTableStack&& st) {
    stack_ = std::move(st.stack_);
    main_table_ = st.main_table_;
    class_table_ = st.class_table_;
    sys_table_ = st.sys_table_;
    pos_func_table_ = st.pos_func_table_;
    pos_class_table_ = st.pos_class_table_;
    pos_lambda_table_ = st.pos_lambda_table_;
  }

  SymbolTableStack& operator=(SymbolTableStack&& st) {
    stack_ = std::move(st.stack_);
    main_table_ = st.main_table_;
    class_table_ = st.class_table_;
    sys_table_ = st.sys_table_;
    pos_func_table_ = st.pos_func_table_;
    pos_class_table_ = st.pos_class_table_;
    pos_lambda_table_ = st.pos_lambda_table_;

    return *this;
  }

  // Insert a table on the stack
  inline void Push(SymbolTablePtr table, bool is_main = false) {
    if (is_main) {
      main_table_ = table;
      return;
    }

    if (table->Type() == SymbolTable::TableType::FUNC_TABLE) {
      pos_func_table_ = stack_.size();
    }

    if (table->Type() == SymbolTable::TableType::CLASS_TABLE) {
      pos_class_table_ = stack_.size();
    }

    if (table->Type() == SymbolTable::TableType::LAMBDA_TABLE) {
      pos_lambda_table_ = stack_.size();
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
  SymbolAttr& Lookup(const std::string& name, bool create,
      bool global = false, bool sys = false);

  inline SymbolAttr& LookupSys(const std::string& name) {
    auto it_obj = sys_table_->Lookup(name);

    if (it_obj != sys_table_->end()) {
      return it_obj->second;
    }

    throw RunTimeError(RunTimeError::ErrorCode::SYMBOL_NOT_FOUND,
        boost::format("symbol %1% not found")% name);
  }

  std::shared_ptr<Object>& LookupFuncRef(const std::string& name, bool create);

  bool Exists(const std::string& name);

  std::tuple<std::shared_ptr<Object>,bool>
  LookupObj(const std::string& name);

  bool Remove(const std::string& name);

  inline bool InsertEntry(const std::string& name, SymbolAttr&& symbol) {
    if (stack_.size() > 0) {
      return stack_.back()->SetValue(name, std::move(symbol));
    }

    return main_table_.lock()->SetValue(name, std::move(symbol));
  }

  inline bool InsertSysEntry(const std::string& name, SymbolAttr&& symbol) {
    return sys_table_->SetValue(name, std::move(symbol));
  }

  void SetEntry(const std::string& name,
                std::shared_ptr<Object> value) {
    if (stack_.size() > 0) {
      stack_.back()->SetValue(name, value);
      return;
    }

    main_table_.lock()->SetValue(name, value);
  }

  std::shared_ptr<CmdEntry> LookupCmd(const std::string& name) {
    std::shared_ptr<CmdEntry> cmd = sys_table_->LookupCmd(name);

    return cmd;
  }

  void SetCmd(const std::string& name, std::shared_ptr<CmdEntry> cmd) {
    sys_table_->SetCmd(name, cmd);
  }

  void SetCmdAlias(const std::string& name, std::vector<std::string>&& cmd) {
    sys_table_->SetCmdAlias(name, std::move(cmd));
  }

  bool ExistsCmdAlias(const std::string& name) const {
    return sys_table_->ExistsCmdAlias(name);
  }

  const std::vector<std::string>& GetCmdAlias(const std::string& name) {
    return sys_table_->GetCmdAlias(name);
  }

  // insert the object on the table stack of the function
  void SetEntryOnFunc(const std::string& name,
                      std::shared_ptr<Object> value) {
    // search the last function table inserted
    for (int i = stack_.size() - 1; i >= 0; i--) {
      if (stack_.at(i)->Type() == SymbolTable::TableType::FUNC_TABLE ||
          stack_.at(i)->Type() == SymbolTable::TableType::LAMBDA_TABLE) {
        stack_.at(i)->SetValue(name, value);
      }
    }
  }

  SymbolTablePtr MainTable() const noexcept {
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

  void SetFirstAsMain() {
    main_table_ = *stack_.begin();
  }

  inline bool HasFuncTable() const noexcept {
    if (pos_func_table_ >= 0) {
      return true;
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

  std::vector<SymbolTablePtr> GetUntilFuncTable() const;

  std::vector<SymbolTablePtr> GetUntilClassTable() const;

  inline void NewClassTable() {
     class_table_ = SymbolTablePtr(new SymbolTable);
  }

  SymbolAttr& LookupClass(const std::string& name);

  SymbolTablePtr& GetClassTable();

  bool ExistsSymbolInClass(const std::string& name);

  inline SysSymbolTable* SysTable() {
    return sys_table_;
  }

  void Dump() {
    std::cout << "*************\n";
    std::cout << "Table: " << this << " Num: " << stack_.size() << " this: " << this << "\n";
    std::cout << "main table copy: " << main_table_.use_count() << "\n";

    main_table_.lock()->Dump();

    for (auto& e: stack_) {
      std::cout << "-- start table: " << e.get() << "\n";
      e->Dump();
      std::cout << "-- end table: " << e.get() << "\n";
    }
    std::cout << "*************\n";
  }

 private:
  std::vector<SymbolTablePtr> stack_;
  std::weak_ptr<SymbolTable> main_table_;
  std::shared_ptr<SymbolTable> class_table_;
  SysSymbolTable* sys_table_;
  int pos_func_table_;
  int pos_class_table_;
  int pos_lambda_table_;
};

}
}

#endif  // SHPP_SYMBOL_TABLE_H
