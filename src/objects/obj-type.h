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

#ifndef SETI_OBJ_TYPE_H
#define SETI_OBJ_TYPE_H

#include <string>
#include <memory>
#include <unordered_map>
#include <tuple>
#include <list>
#include <iostream>

#include "run_time_error.h"
#include "ast/ast.h"
#include "interpreter/symbol-table.h"
#include "abstract-obj.h"
#include "simple-object.h"
#include "func-object.h"
#include "interpreter/interpreter.h"

namespace seti {
namespace internal {

class DeclClassObject: public Object {
 public:
  DeclClassObject(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::DECL_OBJ, obj_type, std::move(sym_table)) {
    symbol_table_stack().NewTable();
  }

  virtual ~DeclClassObject() {
    symbol_table_stack().Pop();
  }

  std::shared_ptr<Object> Attr(std::shared_ptr<Object> self,
                                const std::string& name) override;

  std::shared_ptr<Object>& AttrAssign(std::shared_ptr<Object>,
                                        const std::string& name) override;

  ObjectPtr Add(ObjectPtr obj) override;

  std::string Print() override {
    // TODO: call print function
    return std::string("object");
  }

  SymbolTableStack& SymTable() {
    return symbol_table_stack();
  }

  void SetSelf(ObjectPtr self_obj) {
    self_ = self_obj;
  }

 private:
  std::weak_ptr<Object> self_;
};

class ModuleImportObject: public Object {
 public:
  ModuleImportObject(std::string module_name, bool is_file_path,
                     ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::MODULE, obj_type, std::move(sym_table))
      , interpreter_(false)
      , module_name_(module_name)
      , is_file_path_(is_file_path) {
    try {
      ScriptStream file(module_name_);
      interpreter_.Exec(file);
    } catch (RunTimeError& e) {
      Message msg(Message::Severity::ERR, boost::format(e.msg()), e.pos().line,
                  e.pos().col);

      throw RunTimeError (RunTimeError::ErrorCode::IMPORT,
                          boost::format("import: %1% error")%module_name_)
          .AppendMsg(std::move(msg));
    }
  }

  virtual ~ModuleImportObject() {}

  std::shared_ptr<Object> Attr(std::shared_ptr<Object>/*self*/,
                               const std::string& name) override;

  SymbolTableStack& SymTableStack() {
    return interpreter_.SymTableStack();
  }

  std::string Print() override {
    return std::string("[moule: ") + module_name_ + "]\n";
  }

 private:
  Interpreter interpreter_;
  std::string module_name_;
  bool is_file_path_;
};

class ModuleCustonObject: public Object {
 public:
  using MemberTable = std::vector<std::pair<std::string, ObjectPtr>>;

  ModuleCustonObject(std::string module_name, MemberTable&& member_table,
                     ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::MODULE, obj_type, std::move(sym_table))
      , module_name_(module_name)
      , symbol_table_(SymbolTablePtr(new SymbolTable))
      , symbol_table_stack_(symbol_table_) {
    for (auto& pair: member_table) {
      SymbolAttr sym_entry(pair.second, true);
      symbol_table_stack_.InsertEntry(pair.first, std::move(sym_entry));
    }
  }

  virtual ~ModuleCustonObject() {}

  std::shared_ptr<Object> Attr(std::shared_ptr<Object>/*self*/,
                               const std::string& name) override;

  SymbolTableStack& SymTableStack() {
    return symbol_table_stack();
  }

  std::string Print() override {
    return std::string("[module: ") + module_name_ + "]";
  }

  void RegisterMember(const std::string& fname, ObjectPtr obj) {
    SymbolAttr symbol(obj, true);
    symbol_table_stack_.InsertEntry(fname, std::move(symbol));
  }

 private:
  std::string module_name_;
  SymbolTablePtr symbol_table_;
  SymbolTableStack symbol_table_stack_;
};

class TypeObject: public Object {
 public:
  TypeObject(const std::string& name, ObjectPtr obj_type,
             SymbolTableStack&& sym_table)
      : Object(ObjectType::TYPE, obj_type, std::move(sym_table))
      , name_(name) {
    symbol_table_stack().NewTable();
  }

  virtual ~TypeObject() {}

  std::size_t Hash() const override {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type object has no hash method"));
  }

  bool operator==(const Object& obj) const override {
    if (obj.type() != ObjectType::TYPE) {
      return false;
    }

    const TypeObject& type_obj = static_cast<const TypeObject&>(obj);

    if (name_ == type_obj.name_) {
      return true;
    }

    return false;
  }

  virtual ObjectPtr Constructor(Executor* parent,
                                std::vector<ObjectPtr>&& params) = 0;

  // call a calleble object passing the self object
  // this method is useful to execute member method from objects
  virtual ObjectPtr CallObject(const std::string& name, ObjectPtr self_param);

  const std::string& name() const noexcept {
    return name_;
  }

  virtual bool RegiterMethod(const std::string& name, ObjectPtr obj) {
    SymbolAttr sym_entry(obj, true);
    return symbol_table_stack().InsertEntry(name, std::move(sym_entry));
  }

  std::string Print() override {
    return std::string("<type: ") + name_ + ">";
  }

 private:
  std::string name_;
  std::weak_ptr<Object> parent_;
  std::vector<std::weak_ptr<Object>> interfaces_;
};

class Type: public TypeObject {
 public:
  Type(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("type", obj_type, std::move(sym_table)) {}

  virtual ~Type() {}

  ObjectPtr Constructor(Executor* /*parent*/,
                        std::vector<ObjectPtr>&& params) override;
};

class DeclClassType: public TypeObject {
 public:
  DeclClassType(const std::string& name, ObjectPtr obj_type,
             SymbolTableStack&& sym_table)
      : TypeObject(name, obj_type, std::move(sym_table)) {}

  virtual ~DeclClassType() {}

  bool RegiterMethod(const std::string& name, ObjectPtr obj) override {
    SymbolAttr sym_entry(obj, true);
    return symbol_table_stack().InsertEntry(name, std::move(sym_entry));
  }

  ObjectPtr CallObject(const std::string& name, ObjectPtr self_param) override;

  std::shared_ptr<Object> Attr(std::shared_ptr<Object> self,
                               const std::string& name) override;

  SymbolTableStack& SymTableStack() noexcept {
    return symbol_table_stack();
  }

  ObjectPtr Constructor(Executor* parent,
                        std::vector<ObjectPtr>&& params) override;
};

class NullType: public TypeObject {
 public:
  NullType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("null_t", obj_type, std::move(sym_table)) {}

  virtual ~NullType() {}

  ObjectPtr Constructor(Executor* /*parent*/,
                        std::vector<ObjectPtr>&& params) override;
};

class BoolType: public TypeObject {
 public:
  BoolType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("bool", obj_type, std::move(sym_table)) {}
  virtual ~BoolType() {}

  ObjectPtr Constructor(Executor* /*parent*/,
                        std::vector<ObjectPtr>&& params) override;
};

class IntType: public TypeObject {
 public:
  IntType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("int", obj_type, std::move(sym_table)) {}

  virtual ~IntType() {}

  ObjectPtr Constructor(Executor* /*parent*/,
                        std::vector<ObjectPtr>&& params) override;
};

class RealType: public TypeObject {
 public:
  RealType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("real", obj_type, std::move(sym_table)) {}

  virtual ~RealType() {}

  ObjectPtr Constructor(Executor* /*parent*/,
                        std::vector<ObjectPtr>&& params) override;
};

class SliceType: public TypeObject {
 public:
  SliceType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("slice", obj_type, std::move(sym_table)) {}

  virtual ~SliceType() {}

  ObjectPtr Constructor(Executor*, std::vector<ObjectPtr>&& params) override;
};

class CmdIterType: public TypeObject {
 public:
  CmdIterType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("cmd_iter", obj_type, std::move(sym_table)) {}

  virtual ~CmdIterType() {}

  ObjectPtr Constructor(Executor*, std::vector<ObjectPtr>&&) override;
};

class ArrayIterType: public TypeObject {
 public:
  ArrayIterType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("array_iter", obj_type, std::move(sym_table)) {}

  virtual ~ArrayIterType() {}

  ObjectPtr Constructor(Executor* /*parent*/,
                        std::vector<ObjectPtr>&& params) override;
};

class MapIterType: public TypeObject {
 public:
  MapIterType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("map_iter", obj_type, std::move(sym_table)) {}

  virtual ~MapIterType() {}

  ObjectPtr Constructor(Executor* /*parent*/,
                        std::vector<ObjectPtr>&& params) override;
};


class ContainerType: public TypeObject {
 public:
  ContainerType(const std::string& name, ObjectPtr obj_type,
                SymbolTableStack&& sym_table)
      : TypeObject(name, obj_type, std::move(sym_table)) {}

  virtual ~ContainerType() {}

  virtual ObjectPtr Constructor(Executor* /*parent*/,
                                std::vector<ObjectPtr>&& params) {
    if (params.size() != 1) {
      throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                         boost::format("%1%() takes exactly 1 argument")
                         %name());
    }

    return params[0]->Copy();
  }
};

class MapType: public ContainerType {
 public:
  MapType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : ContainerType("map", obj_type, std::move(sym_table)) {}

  virtual ~MapType() {}
};

class TupleType: public ContainerType {
 public:
  TupleType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : ContainerType("tuple", obj_type, std::move(sym_table)) {}

  ObjectPtr Constructor(Executor* /*parent*/,
                        std::vector<ObjectPtr>&& params) override;

  virtual ~TupleType() {}
};

class FuncType: public TypeObject {
 public:
  FuncType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("function", obj_type, std::move(sym_table)) {}

  virtual ~FuncType() {}

  ObjectPtr Constructor(Executor* /*parent*/,
                        std::vector<ObjectPtr>&& /*params*/) override {
      throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                         boost::format("func() not contructable"));

    return ObjectPtr(nullptr);
  }
};

class ModuleType: public TypeObject {
 public:
  ModuleType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("module", obj_type, std::move(sym_table)) {}

  virtual ~ModuleType() {}

  ObjectPtr Constructor(Executor* /*parent*/,
                        std::vector<ObjectPtr>&& params) override;
};

}
}

#endif  // SETI_OBJ_TYPE_H

