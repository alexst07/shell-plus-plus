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
#include "exceptions-object.h"

namespace shpp {
namespace internal {

// compare string in compile time
constexpr int c_strcmp( char const* lhs, char const* rhs) {
  return (('\0' == lhs[0]) && ('\0' == rhs[0])) ? 0
      :  (lhs[0] != rhs[0]) ? (lhs[0] - rhs[0])
      : c_strcmp( lhs+1, rhs+1 );
}

#define TYPE_OBJECT_EXCPET_FACTORY(NAME, FNAME, OBJ_CLASS, TYPE_CLASS, BASE)   \
  ObjectPtr New ## FNAME(const std::string& msg) {                             \
    auto obj_type = symbol_table_.LookupSys(#NAME).SharedAccess();             \
    return ObjectPtr(new OBJ_CLASS(msg, obj_type, std::move(SymTableStack())));\
  }                                                                            \
                                                                               \
  inline ObjectPtr New ## FNAME ## Type() {                                    \
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();            \
    auto base = 0 == c_strcmp(#BASE, "object")?                                \
      ObjectPtr(nullptr):symbol_table_.LookupSys(#BASE).SharedAccess();        \
    return std::make_shared<TYPE_CLASS>(obj_type, std::move(SymTableStack()),  \
        base);                                                                 \
  }

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

  TYPE_OBJECT_EXCPET_FACTORY(Exception, Exception, ExceptionObject,
      ExceptionType, object)

  TYPE_OBJECT_EXCPET_FACTORY(NullAccessException, NullAccessException,
      NullAccessExceptionObject, NullAccessExceptionType, Exception)

  TYPE_OBJECT_EXCPET_FACTORY(LookupException, LookupException,
      LookupExceptionObject, LookupExceptionType, Exception)

  TYPE_OBJECT_EXCPET_FACTORY(InvalidCmdException, InvalidCmdException,
      InvalidCmdExceptionObject, InvalidCmdExceptionType, Exception)

  TYPE_OBJECT_EXCPET_FACTORY(BadAllocException, BadAllocException,
      BadAllocExceptionObject, BadAllocExceptionType, Exception)

  TYPE_OBJECT_EXCPET_FACTORY(IndexException, IndexException,
      IndexExceptionObject, IndexExceptionType, Exception)

  TYPE_OBJECT_EXCPET_FACTORY(KeyException, KeyException,
      KeyExceptionObject, KeyExceptionType, Exception)

  TYPE_OBJECT_EXCPET_FACTORY(InvalidArgsException, InvalidArgsException,
      InvalidArgsExceptionObject, InvalidArgsExceptionType, Exception)

  TYPE_OBJECT_EXCPET_FACTORY(TypeException, TypeException,
      TypeExceptionObject, TypeExceptionType, Exception)

  TYPE_OBJECT_EXCPET_FACTORY(FuncParamsException, FuncParamsException,
      FuncParamsExceptionObject, FuncParamsExceptionType, Exception)

  TYPE_OBJECT_EXCPET_FACTORY(ZeroDivException, ZeroDivException,
      ZeroDivExceptionObject, ZeroDivExceptionType, Exception)

  TYPE_OBJECT_EXCPET_FACTORY(FdNotFoundException, FdNotFoundException,
      FdNotFoundExceptionObject, FdNotFoundExceptionType, Exception)

  TYPE_OBJECT_EXCPET_FACTORY(IOException, IOException,
      IOExceptionObject, IOExceptionType, Exception)

  TYPE_OBJECT_EXCPET_FACTORY(ImportException, ImportException,
      ImportExceptionObject, ImportExceptionType, Exception)

  TYPE_OBJECT_EXCPET_FACTORY(AssertException, AssertException,
      AssertExceptionObject, AssertExceptionType, Exception)

  TYPE_OBJECT_EXCPET_FACTORY(ParserException, ParserException,
      ParserExceptionObject,ParserExceptionType, Exception)

  TYPE_OBJECT_EXCPET_FACTORY(RegexException, RegexException,
      RegexExceptionObject, RegexExceptionType, Exception)

  TYPE_OBJECT_EXCPET_FACTORY(GlobException, GlobException,
      GlobExceptionObject, GlobExceptionType, Exception)

  TYPE_OBJECT_EXCPET_FACTORY(EvalException, EvalException,
      EvalExceptionObject, EvalExceptionType, Exception)

  TYPE_OBJECT_EXCPET_FACTORY(ErrorException, ErrorException,
      ErrorExceptionObject, ErrorExceptionType, Exception)

  inline ObjectPtr NewRootObject() {
    auto obj_type = symbol_table_.LookupSys("object").SharedAccess();
    return ObjectPtr(new RootObject(obj_type, std::move(SymTableStack())));
  }

  inline ObjectPtr NewNull() {
    auto obj_type = symbol_table_.LookupSys("null_t").SharedAccess();
    return ObjectPtr(new NullObject(obj_type, std::move(SymTableStack())));
  }

  inline ObjectPtr NewBool(bool v) {
    auto obj_type = symbol_table_.LookupSys("bool").SharedAccess();
    return ObjectPtr(new BoolObject(v, obj_type, std::move(SymTableStack())));
  }

  inline ObjectPtr NewInt(int v) {
    auto obj_type = symbol_table_.LookupSys("int").SharedAccess();
    return std::make_shared<IntObject>(v, obj_type, std::move(SymTableStack()));
  }

  inline ObjectPtr NewReal(float v) {
    auto obj_type = symbol_table_.LookupSys("real").SharedAccess();
    return ObjectPtr(new RealObject(v, obj_type, std::move(SymTableStack())));
  }

  inline ObjectPtr NewString(const std::string& str) {
    auto obj_type = symbol_table_.LookupSys("string").SharedAccess();
    return ObjectPtr(new StringObject(str, obj_type,
                                      std::move(SymTableStack())));
  }

  inline ObjectPtr NewString(std::string&& str) {
    auto obj_type = symbol_table_.LookupSys("string").SharedAccess();
    return ObjectPtr(new StringObject(std::move(str), obj_type,
                                      std::move(SymTableStack())));
  }

 inline  ObjectPtr NewCmdObj(int status, std::string&& str_stdout,
      std::string&& str_stderr) {
    auto obj_type = symbol_table_.LookupSys("cmdobj").SharedAccess();
    return ObjectPtr(new CmdObject(status, std::move(str_stdout),
                                   std::move(str_stderr), obj_type,
                                   std::move(SymTableStack())));
  }

  inline ObjectPtr NewSlice(ObjectPtr start, ObjectPtr end, ObjectPtr step) {
    auto obj_type = symbol_table_.LookupSys("slice").SharedAccess();
    return ObjectPtr(new SliceObject(start, end, step, obj_type,
                                     std::move(SymTableStack())));
  }

  inline ObjectPtr NewCmdIter(std::string delim, int outerr, ObjectPtr cmd_obj) {
    auto obj_type = symbol_table_.LookupSys("cmd_iter").SharedAccess();
    return ObjectPtr(new CmdIterObject(delim, outerr, cmd_obj, obj_type,
                                         std::move(SymTableStack())));
  }

  inline ObjectPtr NewFileIter(ObjectPtr file_obj) {
    auto obj_type = symbol_table_.LookupSys("file_iter").SharedAccess();
    return ObjectPtr(new FileIterObject(file_obj, obj_type,
                                        std::move(SymTableStack())));
  }

  inline ObjectPtr NewTuple(std::vector<std::unique_ptr<Object>>&& value) {
    auto obj_type = symbol_table_.LookupSys("tuple").SharedAccess();
    return ObjectPtr(new TupleObject(std::move(value), obj_type,
                                     std::move(SymTableStack())));
  }

  inline ObjectPtr NewTuple(std::vector<std::shared_ptr<Object>>&& value) {
    auto obj_type = symbol_table_.LookupSys("tuple").SharedAccess();
    return ObjectPtr(new TupleObject(std::move(value), obj_type,
                                     std::move(SymTableStack())));
  }

  inline ObjectPtr NewTuple(const std::vector<std::shared_ptr<Object>>& value) {
    std::vector<std::shared_ptr<Object>> v(value.size());
    std::copy (value.begin(), value.end(), v.begin());

    auto obj_type = symbol_table_.LookupSys("tuple").SharedAccess();
    return ObjectPtr(new TupleObject(std::move(v), obj_type,
                                     std::move(SymTableStack())));
  }

  inline ObjectPtr NewArray(std::vector<std::unique_ptr<Object>>&& value) {
    auto obj_type = symbol_table_.LookupSys("array").SharedAccess();
    return ObjectPtr(new ArrayObject(std::move(value), obj_type,
                                     std::move(SymTableStack())));
  }

  inline ObjectPtr NewArray(std::vector<std::shared_ptr<Object>>&& value) {
    auto obj_type = symbol_table_.LookupSys("array").SharedAccess();
    return ObjectPtr(new ArrayObject(std::move(value), obj_type,
                                     std::move(SymTableStack())));
  }

  inline ObjectPtr NewArray(const std::vector<std::shared_ptr<Object>>& value) {
    std::vector<std::shared_ptr<Object>> v(value.size());
    std::copy (value.begin(), value.end(), v.begin());

    auto obj_type = symbol_table_.LookupSys("array").SharedAccess();
    return ObjectPtr(new ArrayObject(std::move(v), obj_type,
                                     std::move(SymTableStack())));
  }

  inline ObjectPtr NewRangeIter(int start, int end, int step) {
    auto obj_type = symbol_table_.LookupSys("range_iter").SharedAccess();
    return ObjectPtr(new RangeIterObject(start, end, step, obj_type,
                                         std::move(SymTableStack())));
  }

  inline ObjectPtr NewArrayIter(ObjectPtr array) {
    auto obj_type = symbol_table_.LookupSys("array_iter").SharedAccess();
    return ObjectPtr(new ArrayIterObject(array, obj_type,
                                         std::move(SymTableStack())));
  }

  inline ObjectPtr NewTupleIter(ObjectPtr array) {
    auto obj_type = symbol_table_.LookupSys("tuple_iter").SharedAccess();
    return ObjectPtr(new TupleIterObject(array, obj_type,
                                         std::move(SymTableStack())));
  }

  inline ObjectPtr NewMapIter(ObjectPtr map) {
    auto obj_type = symbol_table_.LookupSys("map_iter").SharedAccess();
    return ObjectPtr(new MapIterObject(map, obj_type,
                                       std::move(SymTableStack())));
  }

  inline ObjectPtr NewMap(std::vector<std::pair<ObjectPtr, ObjectPtr>>&& value) {
    auto obj_type = symbol_table_.LookupSys("map").SharedAccess();
    return ObjectPtr(new MapObject(std::move(value), obj_type,
                                   std::move(SymTableStack())));
  }

  inline ObjectPtr NewMap() {
    auto obj_type = symbol_table_.LookupSys("map").SharedAccess();
    return ObjectPtr(new MapObject(obj_type, std::move(SymTableStack())));
  }

  inline ObjectPtr NewRegex(const std::string str) {
    auto obj_type = symbol_table_.LookupSys("regex").SharedAccess();
    return ObjectPtr(new RegexObject(str, obj_type,
                                     std::move(SymTableStack())));
  }

  inline ObjectPtr NewPath(const std::string str) {
    auto obj_type = symbol_table_.LookupSys("path").SharedAccess();
    return ObjectPtr(new PathObject(str, obj_type,
                                    std::move(SymTableStack())));
  }

  inline ObjectPtr NewPath(const boost::filesystem::path& path) {
    auto obj_type = symbol_table_.LookupSys("path").SharedAccess();
    return ObjectPtr(new PathObject(path, obj_type,
                                    std::move(SymTableStack())));
  }

  inline ObjectPtr NewFile(const std::string& path, std::ios_base::openmode mode) {
    auto obj_type = symbol_table_.LookupSys("file").SharedAccess();
    return ObjectPtr(new FileObject(path, mode, obj_type,
                                    std::move(SymTableStack())));
  }

  inline ObjectPtr NewModule(const std::string& module_path, bool is_file_path) {
    auto obj_type = symbol_table_.LookupSys("module").SharedAccess();
    return ObjectPtr(new ModuleImportObject(module_path, is_file_path, obj_type,
                                   std::move(SymTableStack())));
  }

  inline ObjectPtr NewModule(const std::string& module,
                      ModuleCustonObject::MemberTable&& table) {
    auto obj_type = symbol_table_.LookupSys("module").SharedAccess();
    return ObjectPtr(new ModuleCustonObject(module, std::move(table), obj_type,
                                            std::move(SymTableStack())));
  }

  inline ObjectPtr NewDeclObject(const std::string& name_type) {
    auto obj_type = symbol_table_.Lookup(name_type, false).SharedAccess();
    SymbolTableStack sym_stack;
    sym_stack.Push(symbol_table_.MainTable(), true);
    sym_stack.Append(symbol_table_);
    sym_stack.NewTable();
    ObjectPtr obj(new DeclClassObject(obj_type, std::move(sym_stack)));
    static_cast<DeclClassObject&>(*obj).SetSelf(obj);
    return obj;
  }

   inline ObjectPtr NewFuncDeclObject(const std::string& id,
      std::shared_ptr<Block> start_node,
      const SymbolTableStack& symbol_table,
      std::vector<std::string>&& params,
      std::unordered_map<std::string, ObjectPtr>&& default_values,
      bool variadic, bool lambda, bool fstatic) {
    auto obj_type = symbol_table_.LookupSys("function").SharedAccess();
    return ObjectPtr(new FuncDeclObject(id, start_node, symbol_table,
                                        std::move(params),
                                        std::move(default_values),
                                        variadic, lambda, fstatic, obj_type,
                                        std::move(SymTableStack())));
  }

  inline ObjectPtr NewWrapperFunc(ObjectPtr func, ObjectPtr self) {
    auto obj_type = symbol_table_.LookupSys("function").SharedAccess();
    return ObjectPtr(new FuncWrapperObject(obj_type, func, self,
                                           std::move(SymTableStack())));
  }

  inline ObjectPtr NewRootObjectType() {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    return std::make_shared<RootObjectType>(obj_type, std::move(SymTableStack()));
  }

  inline ObjectPtr NewNullType() {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    return std::make_shared<NullType>(obj_type, std::move(SymTableStack()));
  }

  inline ObjectPtr NewIntType() {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    return std::make_shared<IntType>(obj_type, std::move(SymTableStack()));
  }

  inline ObjectPtr NewRealType() {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    return std::make_shared<RealType>(obj_type, std::move(SymTableStack()));
  }

  inline ObjectPtr NewBoolType() {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    return std::make_shared<BoolType>(obj_type, std::move(SymTableStack()));
  }

  inline ObjectPtr NewStringType() {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    return std::make_shared<StringType>(obj_type, std::move(SymTableStack()));
  }

  inline ObjectPtr NewCmdType() {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    return std::make_shared<CmdType>(obj_type, std::move(SymTableStack()));
  }

  inline ObjectPtr NewCmdIterType() {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    return std::make_shared<CmdIterType>(obj_type, std::move(SymTableStack()));
  }

  inline ObjectPtr NewFileIterType() {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    return std::make_shared<FileIterType>(obj_type, std::move(SymTableStack()));
  }

  inline ObjectPtr NewArrayType() {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    return std::make_shared<ArrayType>(obj_type, std::move(SymTableStack()));
  }

  inline ObjectPtr NewRangeIterType() {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    return std::make_shared<RangeIterType>(obj_type,
                                           std::move(SymTableStack()));
  }

  inline ObjectPtr NewArrayIterType() {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    return std::make_shared<ArrayIterType>(obj_type,
                                           std::move(SymTableStack()));
  }

  inline ObjectPtr NewTupleIterType() {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    return std::make_shared<TupleIterType>(obj_type,
                                           std::move(SymTableStack()));
  }

  inline ObjectPtr NewMapIterType() {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    return std::make_shared<MapIterType>(obj_type,
                                         std::move(SymTableStack()));
  }

  inline ObjectPtr NewTupleType() {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    return std::make_shared<TupleType>(obj_type, std::move(SymTableStack()));
  }

  inline ObjectPtr NewMapType() {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    return std::make_shared<MapType>(obj_type, std::move(SymTableStack()));
  }

  inline ObjectPtr NewRegexType() {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    return std::make_shared<RegexType>(obj_type, std::move(SymTableStack()));
  }

  inline ObjectPtr NewPathType() {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    return std::make_shared<PathType>(obj_type, std::move(SymTableStack()));
  }

  inline ObjectPtr NewFileType() {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    return std::make_shared<FileType>(obj_type, std::move(SymTableStack()));
  }

  inline ObjectPtr NewModuleType() {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    return std::make_shared<ModuleType>(obj_type, std::move(SymTableStack()));
  }

  inline ObjectPtr NewSliceType() {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    return std::make_shared<SliceType>(obj_type, std::move(SymTableStack()));
  }

  inline ObjectPtr NewFuncType() {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    return std::make_shared<FuncType>(obj_type, std::move(SymTableStack()));
  }

  inline ObjectPtr NewDeclType(const std::string& name_type,
      ObjectPtr base = ObjectPtr(nullptr),
      std::vector<std::shared_ptr<Object>>&& ifaces =
          std::vector<std::shared_ptr<Object>>(),
      bool abstract = false, bool is_false = false) {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    SymbolTableStack table_stack(symbol_table_);
    return ObjectPtr(new DeclClassType(name_type, obj_type,
        std::move(table_stack), base, std::move(ifaces), abstract, is_false));
  }

  inline ObjectPtr NewDeclIFace(const std::string& name_type,
      std::vector<std::shared_ptr<Object>>&& ifaces =
          std::vector<std::shared_ptr<Object>>()) {
    auto obj_type = symbol_table_.LookupSys("type").SharedAccess();
    SymbolTableStack table_stack(symbol_table_);
    return ObjectPtr(new DeclInterface(name_type, obj_type,
        std::move(table_stack), std::move(ifaces)));
  }

  inline ObjectPtr NewType() {
    return ObjectPtr(new Type(ObjectPtr(nullptr), std::move(SymTableStack())));
  }

 private:
  SymbolTableStack& symbol_table_;
};

template<class Fn>
inline void RegisterMethod(const std::string& fname,
    SymbolTableStack& symbol_table, TypeObject& type) {
  SymbolTableStack sym_stack;
  sym_stack.Push(symbol_table.MainTable(), true);
  auto func_type = symbol_table.LookupSys("function").SharedAccess();
  ObjectPtr obj_func(new Fn(func_type, std::move(sym_stack)));
  type.RegiterMethod(fname, obj_func);
}

template<class Fn>
inline void RegisterStaticMethod(const std::string& fname,
    SymbolTableStack& symbol_table, TypeObject& type) {
  RegisterMethod<Fn>(fname, symbol_table, type);
}

template<class Fn>
inline ObjectPtr ObjectMethod(SymbolTableStack& symbol_table) {
  SymbolTableStack sym_stack(symbol_table.MainTable());
  auto func_type = symbol_table.LookupSys("function").SharedAccess();
  ObjectPtr obj(new Fn(func_type, std::move(sym_stack)));
  return obj;
}

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
