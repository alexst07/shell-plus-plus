#include "object-factory.h"

#include <string>
#include <iostream>
#include <boost/variant.hpp>

namespace setti {
namespace internal {

void AlocTypes(SymbolTableStack& symbol_table) {
  ObjectFactory obj_factory(symbol_table);

  ObjectPtr obj_type = obj_factory.NewType();
  SymbolAttr symbol_type(obj_type, true);
  symbol_table.InsertEntry(static_cast<const Type&>(*obj_type).name(),
                           std::move(symbol_type));

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
  symbol_table.InsertEntry(static_cast<const RealType&>(*type_bool).name(),
                           std::move(symbol_bool));


  ObjectPtr type_str = obj_factory.NewStringType();
  SymbolAttr symbol_str(type_str, true);
  symbol_table.InsertEntry(static_cast<const StringType&>(*type_str).name(),
                           std::move(symbol_str));

  ObjectPtr type_array = obj_factory.NewArrayType();
  SymbolAttr symbol_array(type_array, true);
  symbol_table.InsertEntry(static_cast<const ArrayType&>(*type_array).name(),
                           std::move(symbol_array));

  ObjectPtr type_array_iter = obj_factory.NewArrayIterType();
  SymbolAttr symbol_array_iter(type_array_iter, true);
  symbol_table.InsertEntry(static_cast<const ArrayIterType&>(
      *type_array_iter).name(), std::move(symbol_array_iter));

  ObjectPtr type_tuple = obj_factory.NewTupleType();
  SymbolAttr symbol_tuple(type_tuple, true);
  symbol_table.InsertEntry(static_cast<const TupleType&>(*type_tuple).name(),
                           std::move(symbol_tuple));

  ObjectPtr type_map = obj_factory.NewMapType();
  SymbolAttr symbol_map(type_map, true);
  symbol_table.InsertEntry(static_cast<const MapType&>(*type_map).name(),
                           std::move(symbol_map));

  ObjectPtr type_func = obj_factory.NewFuncType();
  SymbolAttr symbol_func(type_func, true);
  symbol_table.InsertEntry(static_cast<const FuncType&>(*type_func).name(),
                           std::move(symbol_func));
}

}
}
