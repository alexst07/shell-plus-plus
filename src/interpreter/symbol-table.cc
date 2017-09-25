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

#include "objects/object-factory.h"

namespace shpp {
namespace internal {

SysSymbolTable *SysSymbolTable::instance_ = 0;

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

SymbolAttr& SymbolTable::Lookup(const std::string& name, bool create) {
  // search on main table if no symbol was found
  auto it_obj = map_.find(name);

  if (it_obj != map_.end()) {
    return it_obj->second;
  }

  if (create) {
    SymbolAttr& ref = SetValue(name, false);
    return ref;
  }

  throw RunTimeError(RunTimeError::ErrorCode::SYMBOL_NOT_FOUND,
                      boost::format("symbol %1% not found")% name);
}

std::map<std::string, std::shared_ptr<Object>> SymbolTable::SymMap() {
  std::map<std::string, ObjectPtr> map;

  for (auto& e: map_) {
    map.insert(std::pair<std::string, ObjectPtr>(e.first,
        e.second.SharedAccess()));
  }

  return map;
}

#define ALOC_TYPE(NAME, FNAME)                                                \
  ObjectPtr type_ ## NAME = obj_factory.New ## FNAME ## Type();               \
  SymbolAttr symbol_ ## NAME(type_ ## NAME, true);                            \
  symbol_table.InsertSysEntry(static_cast<const FNAME ## Type&>(              \
      *type_ ## NAME).name(), std::move(symbol_ ## NAME));

void AlocTypes(SymbolTableStack& symbol_table) {
  if (symbol_table.SysTable()->type_allocated_) {
    return;
  }

  symbol_table.SysTable()->type_allocated_ = true;

  ObjectFactory obj_factory(symbol_table);

  ObjectPtr obj_type = obj_factory.NewType();
  SymbolAttr symbol_type(obj_type, true);
  symbol_table.InsertSysEntry(static_cast<const Type&>(*obj_type).name(),
                           std::move(symbol_type));

  ObjectPtr type_obj_root = obj_factory.NewRootObjectType();
  SymbolAttr symbol_root(type_obj_root, true);
  symbol_table.InsertSysEntry(static_cast<const RootObjectType&>(*type_obj_root)
                           .name(), std::move(symbol_root));

  ObjectPtr type_func = obj_factory.NewFuncType();
  SymbolAttr symbol_func(type_func, true);
  symbol_table.InsertSysEntry(static_cast<const FuncType&>(*type_func).name(),
                           std::move(symbol_func));

  ObjectPtr type_null = obj_factory.NewNullType();
  SymbolAttr symbol_null(type_null, true);
  symbol_table.InsertSysEntry(static_cast<const NullType&>(*type_null).name(),
                           std::move(symbol_null));

  ObjectPtr type_int = obj_factory.NewIntType();
  SymbolAttr symbol_int(type_int, true);
  symbol_table.InsertSysEntry(static_cast<const IntType&>(*type_int).name(),
                           std::move(symbol_int));

  ObjectPtr type_real = obj_factory.NewRealType();
  SymbolAttr symbol_real(type_real, true);
  symbol_table.InsertSysEntry(static_cast<const RealType&>(*type_real).name(),
                           std::move(symbol_real));

  ObjectPtr type_bool = obj_factory.NewBoolType();
  SymbolAttr symbol_bool(type_bool, true);
  symbol_table.InsertSysEntry(static_cast<const BoolType&>(*type_bool).name(),
                           std::move(symbol_bool));

  ObjectPtr type_array = obj_factory.NewArrayType();
  SymbolAttr symbol_array(type_array, true);
  symbol_table.InsertSysEntry(static_cast<const ArrayType&>(*type_array).name(),
                           std::move(symbol_array));

  ObjectPtr type_cmd = obj_factory.NewCmdType();
  SymbolAttr symbol_cmd(type_cmd, true);
  symbol_table.InsertSysEntry(static_cast<const CmdType&>(*type_cmd).name(),
                           std::move(symbol_cmd));

  ObjectPtr type_slice = obj_factory.NewSliceType();
  SymbolAttr symbol_slice(type_slice, true);
  symbol_table.InsertSysEntry(static_cast<const CmdType&>(*type_slice).name(),
                           std::move(symbol_slice));

  ObjectPtr type_cmd_iter = obj_factory.NewCmdIterType();
  SymbolAttr symbol_cmd_iter(type_cmd_iter, true);
  symbol_table.InsertSysEntry(static_cast<const CmdIterType&>(
                           *type_cmd_iter).name(), std::move(symbol_cmd_iter));

  ObjectPtr type_file_iter = obj_factory.NewFileIterType();
  SymbolAttr symbol_file_iter(type_file_iter, true);
  symbol_table.InsertSysEntry(static_cast<const FileIterType&>(
      *type_file_iter).name(), std::move(symbol_file_iter));

  ObjectPtr type_array_iter = obj_factory.NewArrayIterType();
  SymbolAttr symbol_array_iter(type_array_iter, true);
  symbol_table.InsertSysEntry(static_cast<const ArrayIterType&>(
      *type_array_iter).name(), std::move(symbol_array_iter));

  ObjectPtr type_tuple_iter = obj_factory.NewTupleIterType();
  SymbolAttr symbol_tuple_iter(type_tuple_iter, true);
  symbol_table.InsertSysEntry(static_cast<const TupleIterType&>(
      *type_tuple_iter).name(), std::move(symbol_tuple_iter));

  ObjectPtr type_range_iter = obj_factory.NewRangeIterType();
  SymbolAttr symbol_range_iter(type_range_iter, true);
  symbol_table.InsertSysEntry(static_cast<const RangeIterType&>(
      *type_range_iter).name(), std::move(symbol_range_iter));

  ObjectPtr type_map_iter = obj_factory.NewMapIterType();
  SymbolAttr symbol_map_iter(type_map_iter, true);
  symbol_table.InsertSysEntry(static_cast<const MapIterType&>(
      *type_map_iter).name(), std::move(symbol_map_iter));

  ObjectPtr type_tuple = obj_factory.NewTupleType();
  SymbolAttr symbol_tuple(type_tuple, true);
  symbol_table.InsertSysEntry(static_cast<const TupleType&>(*type_tuple).name(),
                           std::move(symbol_tuple));

  ObjectPtr type_map = obj_factory.NewMapType();
  SymbolAttr symbol_map(type_map, true);
  symbol_table.InsertSysEntry(static_cast<const MapType&>(*type_map).name(),
                           std::move(symbol_map));

  ObjectPtr type_regex = obj_factory.NewRegexType();
  SymbolAttr symbol_regex(type_regex, true);
  symbol_table.InsertSysEntry(static_cast<const RegexType&>(*type_regex).name(),
                          std::move(symbol_regex));

  ObjectPtr type_path = obj_factory.NewPathType();
  SymbolAttr symbol_path(type_path, true);
  symbol_table.InsertSysEntry(static_cast<const PathType&>(*type_path).name(),
                          std::move(symbol_path));

  ObjectPtr type_file = obj_factory.NewFileType();
  SymbolAttr symbol_file(type_file, true);
  symbol_table.InsertSysEntry(static_cast<const FileType&>(*type_file).name(),
                          std::move(symbol_file));

  ObjectPtr type_module = obj_factory.NewModuleType();
  SymbolAttr symbol_module(type_module, true);
  symbol_table.InsertSysEntry(static_cast<const ModuleType&>(*type_module).name(),
                           std::move(symbol_module));

  ObjectPtr type_str = obj_factory.NewStringType();
  SymbolAttr symbol_str(type_str, true);
  symbol_table.InsertSysEntry(static_cast<const StringType&>(*type_str).name(),
                           std::move(symbol_str));

  ALOC_TYPE(except, Exception)
  ALOC_TYPE(null_acces_except, NullAccessException)
  ALOC_TYPE(lookup_except, LookupException)
  ALOC_TYPE(invalid_cmd_except, InvalidCmdException)
  ALOC_TYPE(bad_alloc_except, BadAllocException)
  ALOC_TYPE(index_except, IndexException)
  ALOC_TYPE(key_except, KeyException)
  ALOC_TYPE(inv_args_except, InvalidArgsException)
  ALOC_TYPE(type_except, TypeException)
  ALOC_TYPE(func_params_except, FuncParamsException)
  ALOC_TYPE(zero_div_except, ZeroDivException)
  ALOC_TYPE(fd_except, FdNotFoundException)
  ALOC_TYPE(io_except, IOException)
  ALOC_TYPE(import_except, ImportException)
  ALOC_TYPE(assert_except, AssertException)
  ALOC_TYPE(parser_except, ParserException)
  ALOC_TYPE(regex_except, RegexException)
  ALOC_TYPE(glob_except, GlobException)
  ALOC_TYPE(eval_except, EvalException)
  ALOC_TYPE(error_except, ErrorException)

}

SymbolTableStack::SymbolTableStack(SymbolTablePtr symbol_table)
    : sys_table_(SysSymbolTable::instance())
    , pos_func_table_(-1)
    , pos_class_table_(-1)
    , pos_lambda_table_(-1) {
  if (symbol_table) {
    main_table_ = symbol_table;
  }
}

SymbolAttr& SymbolTableStack::Lookup(const std::string& name, bool create,
    bool global, bool sys) {
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

  if (sys) {
    // search on sys table if no symbol was found before
    auto it_obj = sys_table_->Lookup(name);

    if (it_obj != sys_table_->end()) {
      return it_obj->second;
    }
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
