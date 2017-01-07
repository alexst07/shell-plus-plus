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
#include "symbol_table.h"
#include "abstract-obj.h"
#include "simple-object.h"
#include "func_object.h"
#include "intepreter.h"

namespace setti {
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

  void Print() override {
//    std::cout << static_cast<TypeObject&>(*ObjType()).name();
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
      , module_name_(module_name)
      , is_file_path_(is_file_path) {
    interpreter_.Exec(module_name_);
  }

  virtual ~ModuleImportObject() {}

  std::shared_ptr<Object> Attr(std::shared_ptr<Object>/*self*/,
                               const std::string& name) override;

  SymbolTableStack& SymTableStack() {
    return interpreter_.SymTableStack();
  }

  void Print() override {
    std::cout << "MODULE("<< module_name_ << ")\n";
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

  void Print() override {
    std::cout << "MODULE("<< module_name_ << ")\n";
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

class SliceObject: public Object {
 public:
  SliceObject(ObjectPtr obj_start, ObjectPtr obj_end, ObjectPtr obj_type,
              SymbolTableStack&& sym_table)
      : Object(ObjectType::SLICE, obj_type, std::move(sym_table)) {
    if (obj_start->type() != ObjectType::INT ||
        obj_end->type() != ObjectType::INT) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("slice parameter must be integer"));

      IntObject& int_start = static_cast<IntObject&>(*obj_start);
      IntObject& int_end = static_cast<IntObject&>(*obj_end);

      start_ = int_start.value();
      end_ = int_end.value();
    }
  }

  SliceObject(const SliceObject& obj)
      : Object(obj), start_(obj.start_), end_(obj.end_) {}

  virtual ~SliceObject() {}

  SliceObject& operator=(const SliceObject& obj) {
    start_ = obj.start_;
    end_ = obj.end_;
    return *this;
  }

  inline int start() const noexcept { return start_; }
  inline int end() const noexcept { return start_; }

  std::size_t Hash() const override {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("slice object has no hash method"));
  }

  bool operator==(const Object& obj) const override {
    if (obj.type() != ObjectType::SLICE) {
      return false;
    }

    const SliceObject& slice = static_cast<const SliceObject&>(obj);

    bool exp = (start_ == slice.start_) && (end_ == slice.end_);

    return exp;
  }

  void Print() override {
    std::cout << "SLICE: start = " << start_ << ", end = " << end_;
  }

 private:
  int start_;
  int end_;
};

class TupleObject: public Object {
 public:
   TupleObject(std::vector<std::unique_ptr<Object>>&& value,
               ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::TUPLE, obj_type, std::move(sym_table)),
        value_(value.size()) {
     for (size_t i = 0; i < value.size(); i++) {
       Object* obj_ptr = value[i].release();
       value_[i] = std::shared_ptr<Object>(obj_ptr);
     }
   }

   TupleObject(std::vector<std::shared_ptr<Object>>&& value, ObjectPtr obj_type,
               SymbolTableStack&& sym_table)
      : Object(ObjectType::TUPLE, obj_type, std::move(sym_table))
      , value_(std::move(value)) {}

   TupleObject(const TupleObject& obj): Object(obj), value_(obj.value_) {}

   virtual ~TupleObject() {}

   inline std::shared_ptr<Object>& ElementRef(size_t i) {
     return value_.at(i);
   }

   inline std::shared_ptr<Object> Element(size_t i) {
     return value_.at(i);
   }

   inline size_t Size() const noexcept {
     return value_.size();
   }

   inline void set(size_t i, std::unique_ptr<Object> obj) {
     Object* obj_ptr = obj.release();
     value_[i] = std::shared_ptr<Object>(obj_ptr);
   }

   std::size_t Hash() const override {
     if (value_.empty()) {
       throw RunTimeError(RunTimeError::ErrorCode::OUT_OF_RANGE,
                          boost::format("hash of empty tuple is not valid"));
     }

     size_t hash = 0;

     // Executes xor operation with hash of each element of tuple
     for (auto& e: value_) {
       hash ^= e->Hash();
     }

     return hash;
   }

   bool operator==(const Object& obj) const override {
     if (obj.type() != ObjectType::TUPLE) {
       return false;
     }

     const TupleObject& tuple_obj = static_cast<const TupleObject&>(obj);

     // If the tuples have different size, they are different
     if (tuple_obj.value_.size() != value_.size()) {
       return false;
     }

     bool r = true;

     // Test each element on tuple
     for (size_t i = 0; i < value_.size(); i++) {
       r = r && (tuple_obj.value_[i] == value_[i]);
     }

     return r;
   }

   void Print() override {
     std::cout << "TUPLE: ( ";
     for (const auto& e: value_) {
       e->Print();
       std::cout << " ";
     }
     std::cout << ")";
   }

 private:
  std::vector<std::shared_ptr<Object>> value_;
};

class MapObject: public Object {
 public:
  using Map =
      std::unordered_map<size_t, std::vector<std::pair<ObjectPtr, ObjectPtr>>>;

  using Pair = std::pair<size_t, std::vector<std::pair<ObjectPtr, ObjectPtr>>>;

  MapObject(std::vector<std::pair<ObjectPtr, ObjectPtr>>&& value,
            ObjectPtr obj_type, SymbolTableStack&& sym_table);

  MapObject(Map&& value, ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::MAP, obj_type, std::move(sym_table))
      , value_(std::move(value)) {}

  std::size_t Hash() const override {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("map object has no hash method"));
  }

  bool operator==(const Object& obj) const override;

  // Return the reference for an object on the map, if there is no
  // entry for this index, create a new empty with this entry and
  // return its reference
  inline ObjectPtr& ElementRef(ObjectPtr obj_index) {
    if (Exists(obj_index)) {
      size_t hash = obj_index->Hash();
      auto it = value_.find(hash);
      return it->second.back().second;
    } else {
      return Insert_(obj_index);
    }
  }

  // Return a tuple object with the element and a bool object
  std::shared_ptr<Object> Element(ObjectPtr obj_index);

  // Create, this method doesn't do any kind of verification
  // the caller method must check if the entry exists on map or not
  ObjectPtr& Insert_(ObjectPtr obj_index);

  bool Exists(ObjectPtr obj_index);

  void Print() override {
    std::cout << "MAP: { ";
    for (auto& list: value_) {
      for (auto& pair: list.second) {
        std::cout << "(";
        pair.first->Print();
        std::cout << ", ";
        pair.second->Print();
        std::cout << ")";
      }
    }
    std::cout << "} ";
  }

 private:
   Map value_;
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

  void Print() override {
    std::cout << "TYPE(" << name_ << ")";
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

class CmdType: public TypeObject {
 public:
  CmdType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("cmdobj", obj_type, std::move(sym_table)) {}

  virtual ~CmdType() {}

  ObjectPtr Constructor(Executor*, std::vector<ObjectPtr>&&) override;
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

class ContainerType: public TypeObject {
 public:
  ContainerType(const std::string& name, ObjectPtr obj_type,
                SymbolTableStack&& sym_table)
      : TypeObject(name, obj_type, std::move(sym_table)) {}

  virtual ~ContainerType() {}

  ObjectPtr Constructor(Executor* /*parent*/,
                        std::vector<ObjectPtr>&& params) override {
    if (params.size() != 1) {
      throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                         boost::format("%1%() takes exactly 1 argument")
                         %name());
    }

    return params[0]->Copy();
  }
};

class ArrayType: public ContainerType {
 public:
  ArrayType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : ContainerType("array", obj_type, std::move(sym_table)) {}

  virtual ~ArrayType() {}
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

  virtual ~TupleType() {}
};

class FuncType: public TypeObject {
 public:
  FuncType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("func", obj_type, std::move(sym_table)) {}

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

