#ifndef SETI_OBJECT_FACTORY_H
#define SETI_OBJECT_FACTORY_H

#include <string>
#include <memory>
#include <unordered_map>
#include <tuple>
#include <list>

#include "symbol_table.h"
#include "abstract-obj.h"
#include "obj_type.h"
#include "str-object.h"
#include "array-object.h"

namespace setti {
namespace internal {

class ObjectFactory {
 public:
  ObjectFactory(SymbolTableStack& symbol_table)
      :symbol_table_(symbol_table) {}

  SymbolTableStack SymTableStack() {
    // create a symbol table on the start
    SymbolTableStack table_stack(false);
    auto main_tab = symbol_table_.MainTable();
    table_stack.Push(main_tab, true);

    return table_stack;
  }

  ObjectPtr NewNull() {
    auto obj_type = symbol_table_.Lookup("null_t", false).SharedAccess();
    return ObjectPtr(new NullObject(obj_type, std::move(SymTableStack())));
  }

  ObjectPtr NewBool(bool v) {
    auto obj_type = symbol_table_.Lookup("bool", false).SharedAccess();
    return ObjectPtr(new BoolObject(v, obj_type, std::move(SymTableStack())));
  }

  ObjectPtr NewInt(int v) {
    auto obj_type = symbol_table_.Lookup("int", false).SharedAccess();
    return ObjectPtr(new IntObject(v, obj_type, std::move(SymTableStack())));
  }

  ObjectPtr NewReal(float v) {
    auto obj_type = symbol_table_.Lookup("real", false).SharedAccess();
    return ObjectPtr(new RealObject(v, obj_type, std::move(SymTableStack())));
  }

  ObjectPtr NewString(const std::string& str) {
    auto obj_type = symbol_table_.Lookup("string", false).SharedAccess();
    return ObjectPtr(new StringObject(str, obj_type,
                                      std::move(SymTableStack())));
  }

  ObjectPtr NewString(std::string&& str) {
    auto obj_type = symbol_table_.Lookup("string", false).SharedAccess();
    return ObjectPtr(new StringObject(std::move(str), obj_type,
                                      std::move(SymTableStack())));
  }

  ObjectPtr NewTuple(std::vector<std::unique_ptr<Object>>&& value) {
    auto obj_type = symbol_table_.Lookup("tuple", false).SharedAccess();
    return ObjectPtr(new TupleObject(std::move(value), obj_type,
                                     std::move(SymTableStack())));
  }

  ObjectPtr NewTuple(std::vector<std::shared_ptr<Object>>&& value) {
    auto obj_type = symbol_table_.Lookup("tuple", false).SharedAccess();
    return ObjectPtr(new TupleObject(std::move(value), obj_type,
                                     std::move(SymTableStack())));
  }

  ObjectPtr NewArray(std::vector<std::unique_ptr<Object>>&& value) {
    auto obj_type = symbol_table_.Lookup("array", false).SharedAccess();
    return ObjectPtr(new ArrayObject(std::move(value), obj_type,
                                     std::move(SymTableStack())));
  }

  ObjectPtr NewArray(std::vector<std::shared_ptr<Object>>&& value) {
    auto obj_type = symbol_table_.Lookup("array", false).SharedAccess();
    return ObjectPtr(new ArrayObject(std::move(value), obj_type,
                                     std::move(SymTableStack())));
  }

  ObjectPtr NewArrayIter(ObjectPtr array) {
    auto obj_type = symbol_table_.Lookup("array_iter", false).SharedAccess();
    return ObjectPtr(new ArrayIterObject(array, obj_type,
                                         std::move(SymTableStack())));
  }

  ObjectPtr NewMap(std::vector<std::pair<ObjectPtr, ObjectPtr>>&& value) {
    auto obj_type = symbol_table_.Lookup("map", false).SharedAccess();
    return ObjectPtr(new MapObject(std::move(value), obj_type,
                                   std::move(SymTableStack())));
  }

  ObjectPtr NewDeclObject(const std::string& name_type) {
    auto obj_type = symbol_table_.Lookup(name_type, false).SharedAccess();
    SymbolTableStack sym_stack = SymTableStack();
    sym_stack.NewTable();
    return ObjectPtr(new DeclClassObject(obj_type, std::move(sym_stack)));
  }

  ObjectPtr NewFuncDeclObject(const std::string& id, AstNode* start_node,
                              const SymbolTableStack& symbol_table,
                              std::vector<std::string>&& params,
                              std::vector<ObjectPtr>&& default_values,
                              bool variadic) {
    auto obj_type = symbol_table_.Lookup("func", false).SharedAccess();
    return ObjectPtr(new FuncDeclObject(id, start_node, symbol_table,
                                        std::move(params),
                                        std::move(default_values),
                                        variadic, obj_type,
                                        std::move(SymTableStack())));
  }

  ObjectPtr NewWrapperFunc(ObjectPtr func, ObjectPtr self) {
    auto obj_type = symbol_table_.Lookup("func", false).SharedAccess();
    return ObjectPtr(new FuncWrapperObject(obj_type, func, self,
                                           std::move(SymTableStack())));
  }

  ObjectPtr NewNullType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return ObjectPtr(new NullType(obj_type, std::move(SymTableStack())));
  }

  ObjectPtr NewIntType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return ObjectPtr(new IntType(obj_type, std::move(SymTableStack())));
  }

  ObjectPtr NewRealType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return ObjectPtr(new RealType(obj_type, std::move(SymTableStack())));
  }

  ObjectPtr NewBoolType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return ObjectPtr(new BoolType(obj_type, std::move(SymTableStack())));
  }

  ObjectPtr NewStringType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return ObjectPtr(new StringType(obj_type, std::move(SymTableStack())));
  }

  ObjectPtr NewArrayType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return ObjectPtr(new ArrayType(obj_type, std::move(SymTableStack())));
  }

  ObjectPtr NewArrayIterType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return ObjectPtr(new ArrayType(obj_type, std::move(SymTableStack())));
  }

  ObjectPtr NewTupleType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return ObjectPtr(new TupleType(obj_type, std::move(SymTableStack())));
  }

  ObjectPtr NewMapType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return ObjectPtr(new MapType(obj_type, std::move(SymTableStack())));
  }

  ObjectPtr NewFuncType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return ObjectPtr(new FuncType(obj_type, std::move(SymTableStack())));
  }

  ObjectPtr NewDeclType(const std::string& name_type) {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    SymbolTableStack sym_stack = SymTableStack();
    sym_stack.NewTable();
    return ObjectPtr(new DeclClassType(name_type, obj_type,
                                       std::move(sym_stack)));
  }

  ObjectPtr NewType() {
    return ObjectPtr(new Type(ObjectPtr(nullptr), std::move(SymTableStack())));
  }

 private:
  SymbolTableStack& symbol_table_;
};

void AlocTypes(SymbolTableStack& symbol_table);

}
}

#endif  // SETI_OBJECT_FACTORY_H
