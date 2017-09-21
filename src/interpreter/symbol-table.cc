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

#include "symbol-table.h"

namespace shpp {
namespace internal {

SymbolAttr& SymbolTable::SetValue(const std::string& name, bool global) {
  auto it = map_.find(name);
  if (it != map_.end()) {
    return it->second;
  } else {
    it = map_.begin();

    // declare a variable as local
    SymbolAttr symbol(global);
    SymbolIterator it_symbol = map_.insert (it, std::move(
        std::pair<std::string, SymbolAttr>(name, std::move(symbol))));
    return it_symbol->second;
  }
}

void SymbolTable::SetValue(const std::string& name,
    std::shared_ptr<Object> value, bool global) {
  auto it = map_.find(name);
  if (it != map_.end()) {
    it->second.set_value(value);
    return;
  }

  // declare variable always as local
  SymbolAttr symbol(value, global);
  it = map_.begin();
  map_.insert (it, std::move(std::pair<std::string, SymbolAttr>(
      name, std::move(symbol))));
}

bool SymbolTable::SetValue(const std::string& name, SymbolAttr&& symbol) {
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

SymbolAttr& SymbolTableStack::Lookup(const std::string& name, bool create,
    bool global) {
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
      // if the symbol is not on main table, it can't be global
      SymbolAttr& ref = stack_.back()->SetValue(name);
      return ref;
    } else {
      // only symbol on main table can be global
      SymbolAttr& ref = main_table_.lock()->SetValue(name, global);
      return ref;
    }
  }

  throw RunTimeError(RunTimeError::ErrorCode::SYMBOL_NOT_FOUND,
                      boost::format("symbol %1% not found")% name);
}

std::shared_ptr<Object>& SymbolTableStack::LookupFuncRef(
    const std::string& name, bool create) {
  // only modify a variable is it is defined inside function or
  // or if was defined inside class or if this variable was
  // defined as global on root scope

  int stop_point = pos_class_table_ > 1? pos_class_table_:pos_func_table_;
  for (int i = (stack_.size() - 1); i >= stop_point ; i--) {
    auto it_obj = stack_.at(i)->Lookup(name);

    if (it_obj != stack_.at(i)->end()) {
      // return if the symbol was defined inside function
      return it_obj->second.Ref();
    }
  }

  auto it_obj = main_table_.lock()->Lookup(name);

  if (it_obj != main_table_.lock()->end()) {
    if (it_obj->second.global()) {
      // modify a variable on root scope only if it was defined as global
      return it_obj->second.Ref();;
    }
  }

  if (create) {
    SymbolAttr& ref = stack_.back()->SetValue(name);
    return ref.Ref();
  }

  throw RunTimeError(RunTimeError::ErrorCode::SYMBOL_NOT_FOUND,
                      boost::format("symbol %1% not found")% name);
}

bool SymbolTableStack::Exists(const std::string& name) {
  for (int i = (stack_.size() - 1); i >= 0 ; i--) {
    auto it_obj = stack_.at(i)->Lookup(name);

    if (it_obj != stack_.at(i)->end()) {
      return true;
    }
  }

  // search on main table if no symbol was found
  auto it_obj = main_table_.lock()->Lookup(name);

  if (it_obj != main_table_.lock()->end()) {
    return true;
  }

  return false;
}

std::tuple<std::shared_ptr<Object>,bool>
SymbolTableStack::LookupObj(const std::string& name) {
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

bool SymbolTableStack::Remove(const std::string& name) {
  for (int i = (stack_.size() - 1); i >= 0 ; i--) {
    bool r = stack_.at(i)->Remove(name);

    if (r) {
      return r;
    }
  }

  return main_table_.lock()->Remove(name);
}

std::vector<SymbolTablePtr> SymbolTableStack::GetUntilFuncTable() const {
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

std::vector<SymbolTablePtr> SymbolTableStack::GetUntilClassTable() const {
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

SymbolAttr& SymbolTableStack::LookupClass(const std::string& name) {
  // search on main table if no symbol was found
  auto it_obj = class_table_->Lookup(name);

  if (it_obj != class_table_->end()) {
    return it_obj->second;
  }

  SymbolAttr& ref = class_table_->SetValue(name);
  return ref;

  throw RunTimeError(RunTimeError::ErrorCode::SYMBOL_NOT_FOUND,
                      boost::format("symbol '%1%' not found")% name);
}

SymbolTablePtr& SymbolTableStack::GetClassTable() {
  for (auto& table: stack_) {
    if (table->Type() == SymbolTable::TableType::CLASS_TABLE) {
      return table;
    }
  }

  throw RunTimeError(RunTimeError::ErrorCode::SYMBOL_NOT_FOUND,
                      boost::format("can't find class symbol table"));
}

bool SymbolTableStack::ExistsSymbolInClass(const std::string& name) {
  // search on main table if no symbol was found
  auto it_obj = class_table_->Lookup(name);

  if (it_obj != class_table_->end()) {
    return true;
  }

  return false;
}

}
}
