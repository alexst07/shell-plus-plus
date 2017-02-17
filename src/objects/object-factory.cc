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

#include "object-factory.h"

#include <string>
#include <iostream>
#include <boost/variant.hpp>

namespace seti {
namespace internal {

void AlocTypes(SymbolTableStack& symbol_table) {
  ObjectFactory obj_factory(symbol_table);

  ObjectPtr obj_type = obj_factory.NewType();
  SymbolAttr symbol_type(obj_type, true);
  symbol_table.InsertEntry(static_cast<const Type&>(*obj_type).name(),
                           std::move(symbol_type));

  ObjectPtr type_func = obj_factory.NewFuncType();
  SymbolAttr symbol_func(type_func, true);
  symbol_table.InsertEntry(static_cast<const FuncType&>(*type_func).name(),
                           std::move(symbol_func));

  ObjectPtr type_null = obj_factory.NewNullType();
  SymbolAttr symbol_null(type_null, true);
  symbol_table.InsertEntry(static_cast<const NullType&>(*type_null).name(),
                           std::move(symbol_null));

  ObjectPtr type_int = obj_factory.NewIntType();
  SymbolAttr symbol_int(type_int, true);
  symbol_table.InsertEntry(static_cast<const IntType&>(*type_int).name(),
                           std::move(symbol_int));

  ObjectPtr type_real = obj_factory.NewRealType();
  SymbolAttr symbol_real(type_real, true);
  symbol_table.InsertEntry(static_cast<const RealType&>(*type_real).name(),
                           std::move(symbol_real));

  ObjectPtr type_bool = obj_factory.NewBoolType();
  SymbolAttr symbol_bool(type_bool, true);
  symbol_table.InsertEntry(static_cast<const BoolType&>(*type_bool).name(),
                           std::move(symbol_bool));

  ObjectPtr type_array = obj_factory.NewArrayType();
  SymbolAttr symbol_array(type_array, true);
  symbol_table.InsertEntry(static_cast<const ArrayType&>(*type_array).name(),
                           std::move(symbol_array));

  ObjectPtr type_cmd = obj_factory.NewCmdType();
  SymbolAttr symbol_cmd(type_cmd, true);
  symbol_table.InsertEntry(static_cast<const CmdType&>(*type_cmd).name(),
                           std::move(symbol_cmd));

  ObjectPtr type_slice = obj_factory.NewSliceType();
  SymbolAttr symbol_slice(type_slice, true);
  symbol_table.InsertEntry(static_cast<const CmdType&>(*type_slice).name(),
                           std::move(symbol_slice));

  ObjectPtr type_cmd_iter = obj_factory.NewCmdIterType();
  SymbolAttr symbol_cmd_iter(type_cmd_iter, true);
  symbol_table.InsertEntry(static_cast<const CmdIterType&>(
                           *type_cmd_iter).name(), std::move(symbol_cmd_iter));

  ObjectPtr type_array_iter = obj_factory.NewArrayIterType();
  SymbolAttr symbol_array_iter(type_array_iter, true);
  symbol_table.InsertEntry(static_cast<const ArrayIterType&>(
      *type_array_iter).name(), std::move(symbol_array_iter));

  ObjectPtr type_range_iter = obj_factory.NewRangeIterType();
  SymbolAttr symbol_range_iter(type_range_iter, true);
  symbol_table.InsertEntry(static_cast<const RangeIterType&>(
      *type_range_iter).name(), std::move(symbol_range_iter));

  ObjectPtr type_map_iter = obj_factory.NewMapIterType();
  SymbolAttr symbol_map_iter(type_map_iter, true);
  symbol_table.InsertEntry(static_cast<const MapIterType&>(
      *type_map_iter).name(), std::move(symbol_map_iter));

  ObjectPtr type_tuple = obj_factory.NewTupleType();
  SymbolAttr symbol_tuple(type_tuple, true);
  symbol_table.InsertEntry(static_cast<const TupleType&>(*type_tuple).name(),
                           std::move(symbol_tuple));

  ObjectPtr type_map = obj_factory.NewMapType();
  SymbolAttr symbol_map(type_map, true);
  symbol_table.InsertEntry(static_cast<const MapType&>(*type_map).name(),
                           std::move(symbol_map));

  ObjectPtr type_module = obj_factory.NewModuleType();
  SymbolAttr symbol_module(type_module, true);
  symbol_table.InsertEntry(static_cast<const ModuleType&>(*type_module).name(),
                           std::move(symbol_module));

  ObjectPtr type_str = obj_factory.NewStringType();
  SymbolAttr symbol_str(type_str, true);
  symbol_table.InsertEntry(static_cast<const StringType&>(*type_str).name(),
                           std::move(symbol_str));
}

}
}
