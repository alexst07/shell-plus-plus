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

#ifndef SHPP_OBJ_TYPE_H
#define SHPP_OBJ_TYPE_H

#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>

#include "abstract-obj.h"
#include "ast/ast.h"
#include "interpreter/interpreter.h"
#include "interpreter/symbol-table.h"
#include "run_time_error.h"
#include "simple-object.h"

namespace shpp {
namespace internal {

class BaseIter : public Object {
 public:
  ObjectPtr ObjIter(ObjectPtr obj) override { return obj; }

 protected:
  BaseIter(ObjectType type, std::shared_ptr<Object> obj_type,
           SymbolTableStack&& sym_table)
      : Object(type, obj_type, std::move(sym_table)) {}
};

class RangeIterObject : public BaseIter {
 public:
  RangeIterObject(int start, int end, int step, ObjectPtr obj_type,
                  SymbolTableStack&& sym_table);

  virtual ~RangeIterObject() {}

  ObjectPtr Equal(ObjectPtr obj) override;

  ObjectPtr Next() override;

  ObjectPtr HasNext() override;

  std::string Print() override { return std::string("[range_iter]"); }

 private:
  int start_;
  int step_;
  int end_;
  int value_;
};

class Module : public Object {
 public:
  enum class Type { Import, Main, Custon };

  virtual ~Module() {}

  Type ModuleType() { return type_; }

 protected:
  Module(Type type, ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::MODULE, obj_type, std::move(sym_table)),
        type_(type) {}

 private:
  Type type_;
};

class ModuleImportObject : public Module {
 public:
  ModuleImportObject(const std::string& module_path, ObjectPtr obj_type,
                     SymbolTableStack&& sym_table)
      : Module(Module::Type::Import, obj_type, std::move(sym_table)),
        interpreter_(false),
        module_path_(module_path) {}

  virtual ~ModuleImportObject() {}

  std::shared_ptr<Object> Attr(std::shared_ptr<Object> /*self*/,
                               const std::string& name) override;

  SymbolTableStack& SymTableStack() { return interpreter_.SymTableStack(); }

  std::string Print() override {
    return std::string("[moule: ") + module_path_ + "]\n";
  }

  void Execute() try {
    ScriptStream file(module_path_);
    interpreter_.Exec(file);
  } catch (RunTimeError& e) {
    Message msg(Message::Severity::ERR, boost::format(e.msg()), e.pos().line,
                e.pos().col);
    msg.file(module_path_);

    throw RunTimeError(RunTimeError::ErrorCode::IMPORT,
                       boost::format("import: %1% error") % module_path_)
        .AppendMsg(std::move(msg));
  }

 private:
  Interpreter interpreter_;
  std::string module_path_;
};

class ModuleMainObject : public Module {
 public:
  ModuleMainObject(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Module(Module::Type::Main, obj_type, std::move(sym_table)) {}

  virtual ~ModuleMainObject() {}

  std::shared_ptr<Object> Attr(std::shared_ptr<Object> /*self*/,
                               const std::string& name) override;

  SymbolTableStack& SymTableStack() { return symbol_table_stack(); }

  std::string Print() override { return std::string("[moule: MAIN]\n"); }
};

class ModuleCustonObject : public Module {
 public:
  using MemberTable = std::vector<std::pair<std::string, ObjectPtr>>;

  ModuleCustonObject(std::string module_name, MemberTable&& member_table,
                     ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Module(Module::Type::Custon, obj_type, std::move(sym_table)),
        module_name_(module_name),
        symbol_table_(SymbolTablePtr(new SymbolTable)),
        symbol_table_stack_(symbol_table_) {
    for (auto& pair : member_table) {
      SymbolAttr sym_entry(pair.second, true);
      symbol_table_stack_.InsertEntry(pair.first, std::move(sym_entry));
    }
  }

  virtual ~ModuleCustonObject() {}

  std::shared_ptr<Object> Attr(std::shared_ptr<Object> /*self*/,
                               const std::string& name) override;

  SymbolTableStack& SymTableStack() { return symbol_table_stack(); }

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

class TypeObject : public Object {
 public:
  using InterfacesList = std::vector<std::shared_ptr<Object>>;

  TypeObject(const std::string& name, ObjectPtr obj_type,
             SymbolTableStack&& sym_table, ObjectPtr base = ObjectPtr(nullptr),
             InterfacesList&& ifaces = InterfacesList(),
             ObjectType type = ObjectType::TYPE, bool is_final = true)
      : Object(type, obj_type, std::move(sym_table), base, std::move(ifaces)),
        sym_table_(new SymbolTable(SymbolTable::TableType::CLASS_TABLE)),
        name_(name),
        is_final_(is_final) {
    // if some base class was defined, this class must be a type
    if (base) {
      if (base->type() != ObjectType::TYPE) {
        throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                           boost::format("base type '%1%' is not a type") %
                               base->ObjectName());
      }
    }

    symbol_table_stack().Push(sym_table_);
  }

  virtual ~TypeObject() {}

  std::size_t Hash() override {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type object has no hash method"));
  }

  bool operator==(const Object& obj) override {
    if (obj.type() != ObjectType::TYPE) {
      return false;
    }

    const TypeObject& type_obj = static_cast<const TypeObject&>(obj);

    if (name_ == type_obj.name_) {
      return true;
    }

    return false;
  }

  ObjectPtr Equal(ObjectPtr obj) override;

  virtual ObjectPtr Constructor(Executor* parent, Args&& params,
                                KWArgs&& = KWArgs()) = 0;

  // call a calleble object passing the self object
  // this method is useful to execute member method from objects
  virtual ObjectPtr CallObject(const std::string& name, ObjectPtr self_param);

  virtual ObjectPtr CallStaticObject(const std::string& name);

  const std::string& name() const noexcept { return name_; }

  virtual std::string ObjectName() override { return name_; }

  virtual bool RegiterMethod(const std::string& name, ObjectPtr obj) {
    SymbolAttr sym_entry(obj, true);
    return sym_table_->SetValue(name, std::move(sym_entry));
  }

  virtual bool RegisterAttr(const std::string& name, ObjectPtr obj) {
    SymbolAttr sym_entry(obj, true);
    return sym_table_->SetValue(name, std::move(sym_entry));
  }

  virtual bool Declared() const { return false; }

  ObjectPtr SearchAttr(const std::string& name);

  ObjectPtr& SearchAttrRef(const std::string& name);

  bool ExistsAttr(const std::string& name);

  bool is_final() const { return is_final_; }

  inline SymbolTablePtr& SymTable() { return sym_table_; }

  std::string Print() override { return std::string("<type: ") + name_ + ">"; }

 private:
  SymbolTablePtr sym_table_;
  std::string name_;
  std::weak_ptr<Object> parent_;
  std::vector<std::weak_ptr<Object>> interfaces_;
  bool is_final_;
};

class Type : public TypeObject {
 public:
  Type(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("type", obj_type, std::move(sym_table)) {}

  virtual ~Type() {}

  ObjectPtr Constructor(Executor*, Args&& params, KWArgs&&) override;
};

class NullType : public TypeObject {
 public:
  NullType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("null_t", obj_type, std::move(sym_table)) {}

  virtual ~NullType() {}

  ObjectPtr Constructor(Executor*, Args&& params, KWArgs&&) override;
};

class BoolType : public TypeObject {
 public:
  BoolType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("bool", obj_type, std::move(sym_table)) {}
  virtual ~BoolType() {}

  ObjectPtr Constructor(Executor*, Args&& params, KWArgs&&) override;
};

class IntType : public TypeObject {
 public:
  IntType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("int", obj_type, std::move(sym_table)) {}

  virtual ~IntType() {}

  ObjectPtr Constructor(Executor*, Args&& params, KWArgs&&) override;
};

class RealType : public TypeObject {
 public:
  RealType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("real", obj_type, std::move(sym_table)) {}

  virtual ~RealType() {}

  ObjectPtr Constructor(Executor*, Args&& params, KWArgs&&) override;
};

class SliceType : public TypeObject {
 public:
  SliceType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("slice", obj_type, std::move(sym_table)) {}

  virtual ~SliceType() {}

  ObjectPtr Constructor(Executor*, Args&& params, KWArgs&&) override;
};

class CmdIterType : public TypeObject {
 public:
  CmdIterType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("cmd_iter", obj_type, std::move(sym_table)) {}

  virtual ~CmdIterType() {}

  ObjectPtr Constructor(Executor*, Args&&, KWArgs&&) override;
};

class GlobIterType : public TypeObject {
 public:
  GlobIterType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("glob_iter", obj_type, std::move(sym_table)) {}

  virtual ~GlobIterType() {}

  ObjectPtr Constructor(Executor*, Args&&, KWArgs&&) override;
};

class RangeIterType : public TypeObject {
 public:
  RangeIterType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("range_iter", obj_type, std::move(sym_table)) {}

  virtual ~RangeIterType() {}

  ObjectPtr Constructor(Executor*, Args&& params, KWArgs&& = KWArgs()) override;
};

class ArrayIterType : public TypeObject {
 public:
  ArrayIterType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("array_iter", obj_type, std::move(sym_table)) {}

  virtual ~ArrayIterType() {}

  ObjectPtr Constructor(Executor*, Args&& params, KWArgs&&) override;
};

class MapIterType : public TypeObject {
 public:
  MapIterType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("map_iter", obj_type, std::move(sym_table)) {}

  virtual ~MapIterType() {}

  ObjectPtr Constructor(Executor*, Args&& params, KWArgs&&) override;
};

class ContainerType : public TypeObject {
 public:
  ContainerType(const std::string& name, ObjectPtr obj_type,
                SymbolTableStack&& sym_table)
      : TypeObject(name, obj_type, std::move(sym_table)) {}

  virtual ~ContainerType() {}

  virtual ObjectPtr Constructor(Executor*, Args&& params, KWArgs&&) {
    if (params.size() != 1) {
      throw RunTimeError(
          RunTimeError::ErrorCode::FUNC_PARAMS,
          boost::format("%1%() takes exactly 1 argument") % name());
    }

    return params[0]->Copy();
  }
};

class TupleType : public ContainerType {
 public:
  TupleType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : ContainerType("tuple", obj_type, std::move(sym_table)) {}

  ObjectPtr Constructor(Executor*, Args&& params, KWArgs&&) override;

  virtual ~TupleType() {}
};

class ModuleType : public TypeObject {
 public:
  ModuleType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("module", obj_type, std::move(sym_table)) {}

  virtual ~ModuleType() {}

  ObjectPtr Constructor(Executor*, Args&& params, KWArgs&&) override;
};

class RootObjectType : public TypeObject {
 public:
  RootObjectType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("object", obj_type, std::move(sym_table)) {}

  virtual ~RootObjectType() {}

  ObjectPtr BaseType() noexcept override { return ObjectPtr(nullptr); }

  ObjectPtr Constructor(Executor*, Args&& params, KWArgs&&) override;
};

bool InstanceOf(ObjectPtr obj, ObjectPtr base);

}  // namespace internal
}  // namespace shpp

#endif  // SHPP_OBJ_TYPE_H
