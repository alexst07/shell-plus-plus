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

namespace setti {
namespace internal {

class NullObject: public Object {
 public:
  NullObject(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::NIL, obj_type, std::move(sym_table)) {}

  virtual ~NullObject() {}

  std::size_t Hash() const override {
    throw RunTimeError(RunTimeError::ErrorCode::NULL_ACCESS,
                       boost::format("null object has no hash method"));
  }

  bool operator==(const Object& obj) const override {
    if (obj.type() == ObjectType::NIL) {
      return true;
    }

    return false;
  }

  void Print() override {
    std::cout << "NIL";
  }

  inline nullptr_t value() const noexcept { return nullptr; }
};

class IntObject: public Object {
 public:
  IntObject(int value, ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::INT, obj_type, std::move(sym_table))
      , value_(value) {}

  IntObject(const IntObject& obj): Object(obj), value_(obj.value_) {}

  virtual ~IntObject() {}

  IntObject& operator=(const IntObject& obj) {
    value_ = obj.value_;
    return *this;
  }

  inline int value() const noexcept { return value_; }

  std::size_t Hash() const override {
    std::hash<int> int_hash;
    return int_hash(value_);
  }

  bool operator==(const Object& obj) const override {
    if (obj.type() != ObjectType::INT) {
      return false;
    }

    int value = static_cast<const IntObject&>(obj).value_;

    return value_ == value;
  }

  void Print() override {
    std::cout << "INT: " << value_;
  }

 private:
  int value_;
};

class BoolObject: public Object {
 public:
  BoolObject(bool value, ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::BOOL, obj_type, std::move(sym_table))
      , value_(value) {}

  BoolObject(const BoolObject& obj): Object(obj), value_(obj.value_) {}

  virtual ~BoolObject() {}

  BoolObject& operator=(const BoolObject& obj) {
    value_ = obj.value_;
    return *this;
  }

  inline bool value() const noexcept { return value_; }

  std::size_t Hash() const override {
    std::hash<bool> bool_hash;
    return bool_hash(value_);
  }

  bool operator==(const Object& obj) const override {
    if (obj.type() != ObjectType::BOOL) {
      return false;
    }

    bool value = static_cast<const BoolObject&>(obj).value_;

    return value_ == value;
  }

  void Print() override {
    std::cout << "BOOL: " << value_;
  }

 private:
  bool value_;
};

class RealObject: public Object {
 public:
  RealObject(float value, ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::REAL, obj_type, std::move(sym_table))
      , value_(value) {}

  RealObject(const RealObject& obj): Object(obj), value_(obj.value_) {}

  virtual ~RealObject() {}

  RealObject& operator=(const RealObject& obj) {
    value_ = obj.value_;
    return *this;
  }

  inline float value() const noexcept { return value_; }

  std::size_t Hash() const override {
    std::hash<float> float_hash;
    return float_hash(value_);
  }

  bool operator==(const Object& obj) const override {
    if (obj.type() != ObjectType::REAL) {
      return false;
    }

    float value = static_cast<const RealObject&>(obj).value_;

    return value_ == value;
  }

  void Print() override {
    std::cout << "REAL: " << value_;
  }

 private:
  float value_;
};

class StringObject: public Object {
 public:
  StringObject(std::string&& value, ObjectPtr obj_type,
               SymbolTableStack&& sym_table)
      : Object(ObjectType::STRING, obj_type, std::move(sym_table))
      , value_(std::move(value)) {}

  StringObject(const std::string& value, ObjectPtr obj_type,
               SymbolTableStack&& sym_table)
      : Object(ObjectType::STRING, obj_type, std::move(sym_table))
      , value_(value) {}

  StringObject(const StringObject& obj): Object(obj), value_(obj.value_) {}

  virtual ~StringObject() {}

  StringObject& operator=(const StringObject& obj) {
    value_ = obj.value_;
    return *this;
  }

  inline const std::string& value() const noexcept { return value_; }

  std::size_t Hash() const override {
    std::hash<std::string> str_hash;
    return str_hash(value_);
  }

  bool operator==(const Object& obj) const override {
    if (obj.type() != ObjectType::STRING) {
      return false;
    }

    std::string value = static_cast<const StringObject&>(obj).value_;

    return value_ == value;
  }

  void Print() override {
    std::cout << "STRING: " << value_;
  }

 private:
  std::string value_;
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

class ArrayObject: public Object {
 public:
   ArrayObject(std::vector<std::unique_ptr<Object>>&& value,
               ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::ARRAY, obj_type, std::move(sym_table))
      , value_(value.size()) {
     for (size_t i = 0; i < value.size(); i++) {
       Object* obj_ptr = value[i].release();
       value_[i] = std::shared_ptr<Object>(obj_ptr);
     }
   }

   ArrayObject(std::vector<std::shared_ptr<Object>>&& value, ObjectPtr obj_type,
               SymbolTableStack&& sym_table)
      : Object(ObjectType::ARRAY, obj_type, std::move(sym_table))
      , value_(value) {}

   ArrayObject(const ArrayObject& obj): Object(obj), value_(obj.value_) {}

   virtual ~ArrayObject() {}

   inline Object* at(size_t i) {
     return value_.at(i).get();
   }

   inline std::shared_ptr<Object>& ElementRef(size_t i) {
     return value_.at(i);
   }

   inline std::shared_ptr<Object> Element(size_t i) {
     return value_.at(i);
   }

   inline void set(size_t i, std::unique_ptr<Object> obj) {
     Object* obj_ptr = obj.release();
     value_[i] = std::shared_ptr<Object>(obj_ptr);
   }

   std::size_t Hash() const override;

   bool operator==(const Object& obj) const override;

   void Print() override {
     std::cout << "ARRAY: [ ";
     for (const auto& e: value_) {
       e->Print();
       std::cout << " ";
     }
     std::cout << "]";
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

class FuncObject: public Object {
 public:
  FuncObject(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::FUNC, obj_type, std::move(sym_table)) {}

  virtual ~FuncObject() {}

  std::size_t Hash() const override {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("func object has no hash method"));
  }

  bool operator==(const Object& obj) const override {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("func object has no compare method"));
  }

  virtual ObjectPtr Call(Executor* parent, std::vector<ObjectPtr>&& params) = 0;

  void Print() override {
    std::cout << "FUNC";
  }
};

class FuncDeclObject: public FuncObject {
 public:
  FuncDeclObject(const std::string& id, AstNode* start_node,
                 const SymbolTableStack& symbol_table,
                 std::vector<std::string>&& params,
                 std::vector<ObjectPtr>&& default_values,
                 bool variadic, ObjectPtr obj_type,
                 SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , id_(id)
      , start_node_(start_node)
      , symbol_table_(true)
      , params_(std::move(params))
      , default_values_(std::move(default_values))
      , variadic_(variadic) {
    symbol_table_.Push(symbol_table.MainTable());
    SymbolTablePtr table = SymbolTable::Create();

    // main symbol of function
    symbol_table_.Push(table, true);
  }

  ObjectPtr Call(Executor* parent, std::vector<ObjectPtr>&& params) override;

 private:
  std::string id_;
  AstNode* start_node_;
  SymbolTableStack symbol_table_;
  std::vector<std::string> params_;
  std::vector<ObjectPtr> default_values_;
  bool variadic_;
};

class TypeObject: public Object {
 public:
  TypeObject(const std::string& name, ObjectPtr obj_type,
             SymbolTableStack&& sym_table)
    : Object(ObjectType::TYPE, obj_type, std::move(sym_table)), name_(name) {}

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

  const std::string& name() const noexcept {
    return name_;
  }

  void Print() override {
    std::cout << "TYPE(" << name_ << ")";
  }

 private:
  std::string name_;
};

class Type: public TypeObject {
 public:
  Type(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("type", obj_type, std::move(sym_table)) {}

  virtual ~Type() {}

  ObjectPtr Constructor(Executor* /*parent*/,
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

 private:
  int Type2Int(float v) {
    return static_cast<int>(v);
  }

  int Type2Int(const std::string& v) {
    try {
      return std::stoi(v);
    } catch (std::exception&) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("invalid string to int"));
    }
  }
};

class RealType: public TypeObject {
 public:
  RealType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("real", obj_type, std::move(sym_table)) {}

  virtual ~RealType() {}

  ObjectPtr Constructor(Executor* /*parent*/,
                        std::vector<ObjectPtr>&& params) override;

 private:
  float Type2Real(int v) {
    return static_cast<float>(v);
  }

  float Type2Real(const std::string& v) {
    try {
      return std::stof(v);
    } catch (std::exception&) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("invalid string to int"));
    }
  }
};

class StringType: public TypeObject {
 public:
  StringType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("string", obj_type, std::move(sym_table)) {}

  virtual ~StringType() {}

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

}
}

#endif  // SETI_OBJ_TYPE_H

