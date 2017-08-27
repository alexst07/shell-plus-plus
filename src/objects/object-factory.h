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

#ifndef SHPP_OBJECT_FACTORY_H
#define SHPP_OBJECT_FACTORY_H

#include <string>
#include <memory>
#include <unordered_map>
#include <tuple>
#include <list>

#include "interpreter/symbol-table.h"
#include "abstract-obj.h"
#include "obj-type.h"
#include "str-object.h"
#include "array-object.h"
#include "cmd-object.h"
#include "map-object.h"
#include "tuple-object.h"
#include "regex.h"
#include "path.h"
#include "file-object.h"
#include "decl-class-object.h"

namespace shpp {
namespace internal {

class ObjectFactory {
 public:
  ObjectFactory(SymbolTableStack& symbol_table)
      :symbol_table_(symbol_table) {}

  SymbolTableStack SymTableStack() {
    // create a symbol table on the start
    SymbolTableStack table_stack;
    auto main_tab = symbol_table_.MainTable();
    table_stack.Push(main_tab, true);

    return table_stack;
  }

  ObjectPtr NewRootObject() {
    auto obj_type = symbol_table_.Lookup("object", false).SharedAccess();
    return ObjectPtr(new RootObject(obj_type, std::move(SymTableStack())));
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
    return std::make_shared<IntObject>(v, obj_type, std::move(SymTableStack()));
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

  ObjectPtr NewCmdObj(int status, std::string&& str_stdout,
                      std::string&& str_stderr) {
    auto obj_type = symbol_table_.Lookup("cmdobj", false).SharedAccess();
    return ObjectPtr(new CmdObject(status, std::move(str_stdout),
                                   std::move(str_stderr), obj_type,
                                   std::move(SymTableStack())));
  }

  ObjectPtr NewSlice(ObjectPtr start, ObjectPtr end, ObjectPtr step) {
    auto obj_type = symbol_table_.Lookup("slice", false).SharedAccess();
    return ObjectPtr(new SliceObject(start, end, step, obj_type,
                                     std::move(SymTableStack())));
  }

  ObjectPtr NewCmdIter(std::string delim, int outerr, ObjectPtr cmd_obj) {
    auto obj_type = symbol_table_.Lookup("cmd_iter", false).SharedAccess();
    return ObjectPtr(new CmdIterObject(delim, outerr, cmd_obj, obj_type,
                                         std::move(SymTableStack())));
  }

  ObjectPtr NewFileIter(ObjectPtr file_obj) {
    auto obj_type = symbol_table_.Lookup("file_iter", false).SharedAccess();
    return ObjectPtr(new FileIterObject(file_obj, obj_type,
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

  ObjectPtr NewTuple(const std::vector<std::shared_ptr<Object>>& value) {
    std::vector<std::shared_ptr<Object>> v(value.size());
    std::copy (value.begin(), value.end(), v.begin());

    auto obj_type = symbol_table_.Lookup("tuple", false).SharedAccess();
    return ObjectPtr(new TupleObject(std::move(v), obj_type,
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

  ObjectPtr NewArray(const std::vector<std::shared_ptr<Object>>& value) {
    std::vector<std::shared_ptr<Object>> v(value.size());
    std::copy (value.begin(), value.end(), v.begin());

    auto obj_type = symbol_table_.Lookup("array", false).SharedAccess();
    return ObjectPtr(new ArrayObject(std::move(v), obj_type,
                                     std::move(SymTableStack())));
  }

  ObjectPtr NewRangeIter(int start, int end, int step) {
    auto obj_type = symbol_table_.Lookup("range_iter", false).SharedAccess();
    return ObjectPtr(new RangeIterObject(start, end, step, obj_type,
                                         std::move(SymTableStack())));
  }

  ObjectPtr NewArrayIter(ObjectPtr array) {
    auto obj_type = symbol_table_.Lookup("array_iter", false).SharedAccess();
    return ObjectPtr(new ArrayIterObject(array, obj_type,
                                         std::move(SymTableStack())));
  }

  ObjectPtr NewTupleIter(ObjectPtr array) {
    auto obj_type = symbol_table_.Lookup("tuple_iter", false).SharedAccess();
    return ObjectPtr(new TupleIterObject(array, obj_type,
                                         std::move(SymTableStack())));
  }

  ObjectPtr NewMapIter(ObjectPtr map) {
    auto obj_type = symbol_table_.Lookup("map_iter", false).SharedAccess();
    return ObjectPtr(new MapIterObject(map, obj_type,
                                       std::move(SymTableStack())));
  }

  ObjectPtr NewMap(std::vector<std::pair<ObjectPtr, ObjectPtr>>&& value) {
    auto obj_type = symbol_table_.Lookup("map", false).SharedAccess();
    return ObjectPtr(new MapObject(std::move(value), obj_type,
                                   std::move(SymTableStack())));
  }

  ObjectPtr NewMap() {
    auto obj_type = symbol_table_.Lookup("map", false).SharedAccess();
    return ObjectPtr(new MapObject(obj_type, std::move(SymTableStack())));
  }

  ObjectPtr NewRegex(const std::string str) {
    auto obj_type = symbol_table_.Lookup("regex", false).SharedAccess();
    return ObjectPtr(new RegexObject(str, obj_type,
                                     std::move(SymTableStack())));
  }

  ObjectPtr NewPath(const std::string str) {
    auto obj_type = symbol_table_.Lookup("path", false).SharedAccess();
    return ObjectPtr(new PathObject(str, obj_type,
                                    std::move(SymTableStack())));
  }

  ObjectPtr NewPath(const boost::filesystem::path& path) {
    auto obj_type = symbol_table_.Lookup("path", false).SharedAccess();
    return ObjectPtr(new PathObject(path, obj_type,
                                    std::move(SymTableStack())));
  }

  ObjectPtr NewFile(const std::string& path, std::ios_base::openmode mode) {
    auto obj_type = symbol_table_.Lookup("file", false).SharedAccess();
    return ObjectPtr(new FileObject(path, mode, obj_type,
                                    std::move(SymTableStack())));
  }

  ObjectPtr NewModule(const std::string& module, const std::string& path,
      bool is_file_path) {
    auto obj_type = symbol_table_.Lookup("module", false).SharedAccess();
    return ObjectPtr(new ModuleImportObject(module, path, is_file_path, obj_type,
                                   std::move(SymTableStack())));
  }

  ObjectPtr NewModule(const std::string& module,
                      ModuleCustonObject::MemberTable&& table) {
    auto obj_type = symbol_table_.Lookup("module", false).SharedAccess();
    return ObjectPtr(new ModuleCustonObject(module, std::move(table), obj_type,
                                            std::move(SymTableStack())));
  }

  ObjectPtr NewDeclObject(const std::string& name_type) {
    auto obj_type = symbol_table_.Lookup(name_type, false).SharedAccess();
    SymbolTableStack sym_stack;
    sym_stack.Push(symbol_table_.MainTable(), true);
    sym_stack.Append(symbol_table_);
    sym_stack.NewTable();
    ObjectPtr obj(new DeclClassObject(obj_type, std::move(sym_stack)));
    static_cast<DeclClassObject&>(*obj).SetSelf(obj);
    return obj;
  }

  ObjectPtr NewFuncDeclObject(const std::string& id,
      std::shared_ptr<Block> start_node,
      const SymbolTableStack& symbol_table,
      std::vector<std::string>&& params,
      std::unordered_map<std::string, ObjectPtr>&& default_values,
      bool variadic, bool lambda, bool fstatic) {
    auto obj_type = symbol_table_.Lookup("function", false).SharedAccess();
    return ObjectPtr(new FuncDeclObject(id, start_node, symbol_table,
                                        std::move(params),
                                        std::move(default_values),
                                        variadic, lambda, fstatic, obj_type,
                                        std::move(SymTableStack())));
  }

  ObjectPtr NewWrapperFunc(ObjectPtr func, ObjectPtr self) {
    auto obj_type = symbol_table_.Lookup("function", false).SharedAccess();
    return ObjectPtr(new FuncWrapperObject(obj_type, func, self,
                                           std::move(SymTableStack())));
  }

  ObjectPtr NewRootObjectType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return std::make_shared<RootObjectType>(obj_type, std::move(SymTableStack()));
  }

  ObjectPtr NewNullType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return std::make_shared<NullType>(obj_type, std::move(SymTableStack()));
  }

  ObjectPtr NewIntType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return std::make_shared<IntType>(obj_type, std::move(SymTableStack()));
  }

  ObjectPtr NewRealType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return std::make_shared<RealType>(obj_type, std::move(SymTableStack()));
  }

  ObjectPtr NewBoolType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return std::make_shared<BoolType>(obj_type, std::move(SymTableStack()));
  }

  ObjectPtr NewStringType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return std::make_shared<StringType>(obj_type, std::move(SymTableStack()));
  }

  ObjectPtr NewCmdType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return std::make_shared<CmdType>(obj_type, std::move(SymTableStack()));
  }

  ObjectPtr NewCmdIterType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return std::make_shared<CmdIterType>(obj_type, std::move(SymTableStack()));
  }

  ObjectPtr NewFileIterType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return std::make_shared<FileIterType>(obj_type, std::move(SymTableStack()));
  }

  ObjectPtr NewArrayType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return std::make_shared<ArrayType>(obj_type, std::move(SymTableStack()));
  }

  ObjectPtr NewRangeIterType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return std::make_shared<RangeIterType>(obj_type,
                                           std::move(SymTableStack()));
  }

  ObjectPtr NewArrayIterType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return std::make_shared<ArrayIterType>(obj_type,
                                           std::move(SymTableStack()));
  }

  ObjectPtr NewTupleIterType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return std::make_shared<TupleIterType>(obj_type,
                                           std::move(SymTableStack()));
  }

  ObjectPtr NewMapIterType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return std::make_shared<MapIterType>(obj_type,
                                         std::move(SymTableStack()));
  }

  ObjectPtr NewTupleType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return std::make_shared<TupleType>(obj_type, std::move(SymTableStack()));
  }

  ObjectPtr NewMapType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return std::make_shared<MapType>(obj_type, std::move(SymTableStack()));
  }

  ObjectPtr NewRegexType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return std::make_shared<RegexType>(obj_type, std::move(SymTableStack()));
  }

  ObjectPtr NewPathType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return std::make_shared<PathType>(obj_type, std::move(SymTableStack()));
  }

  ObjectPtr NewFileType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return std::make_shared<FileType>(obj_type, std::move(SymTableStack()));
  }

  ObjectPtr NewModuleType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return std::make_shared<ModuleType>(obj_type, std::move(SymTableStack()));
  }

  ObjectPtr NewSliceType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return std::make_shared<SliceType>(obj_type, std::move(SymTableStack()));
  }

  ObjectPtr NewFuncType() {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    return std::make_shared<FuncType>(obj_type, std::move(SymTableStack()));
  }

  ObjectPtr NewDeclType(const std::string& name_type,
      ObjectPtr base = ObjectPtr(nullptr),
      std::vector<std::shared_ptr<Object>>&& ifaces =
          std::vector<std::shared_ptr<Object>>(),
      bool abstract = false) {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    SymbolTableStack table_stack(symbol_table_);
    return ObjectPtr(new DeclClassType(name_type, obj_type,
        std::move(table_stack), base, std::move(ifaces), abstract));
  }

  ObjectPtr NewDeclIFace(const std::string& name_type,
      std::vector<std::shared_ptr<Object>>&& ifaces =
          std::vector<std::shared_ptr<Object>>()) {
    auto obj_type = symbol_table_.Lookup("type", false).SharedAccess();
    SymbolTableStack table_stack(symbol_table_);
    return ObjectPtr(new DeclInterface(name_type, obj_type,
        std::move(table_stack), std::move(ifaces)));
  }

  ObjectPtr NewType() {
    return ObjectPtr(new Type(ObjectPtr(nullptr), std::move(SymTableStack())));
  }

 private:
  SymbolTableStack& symbol_table_;
};

template<class Fn>
void RegisterMethod(const std::string& fname, SymbolTableStack& symbol_table,
                    TypeObject& type) {
  SymbolTableStack sym_stack;
  sym_stack.Push(symbol_table.MainTable(), true);
  auto func_type = symbol_table.Lookup("function", false).SharedAccess();
  ObjectPtr obj_func(new Fn(func_type, std::move(sym_stack)));
  type.RegiterMethod(fname, obj_func);
}

template<class Fn>
void RegisterStaticMethod(const std::string& fname,
                          SymbolTableStack& symbol_table,
                          TypeObject& type) {
  RegisterMethod<Fn>(fname, symbol_table, type);
}

template<class Fn>
ObjectPtr ObjectMethod(SymbolTableStack& symbol_table) {
  SymbolTableStack sym_stack(symbol_table.MainTable());
  auto func_type = symbol_table.Lookup("function", false).SharedAccess();
  ObjectPtr obj(new Fn(func_type, std::move(sym_stack)));
  return obj;
}

void AlocTypes(SymbolTableStack& symbol_table);

// Pass the variable as value or reference depending on type
inline ObjectPtr PassVar(ObjectPtr obj, SymbolTableStack& symbol_table_stack) {
  ObjectFactory obj_factory(symbol_table_stack);

  if (!obj) {
    return obj_factory.NewNull();
  }

  switch (obj->type()) {
    case Object::ObjectType::NIL:
      return obj_factory.NewNull();
      break;

    case Object::ObjectType::INT:
      return obj_factory.NewInt(static_cast<IntObject&>(*obj).value());
      break;

    case Object::ObjectType::BOOL:
      return obj_factory.NewBool(static_cast<BoolObject&>(*obj).value());
      break;

    case Object::ObjectType::REAL:
      return obj_factory.NewReal(static_cast<RealObject&>(*obj).value());
      break;

    case Object::ObjectType::STRING:
      return obj_factory.NewString(static_cast<StringObject&>(*obj).value());
      break;

    default:
      return obj;
  }
}

}
}

#endif  // SHPP_OBJECT_FACTORY_H
