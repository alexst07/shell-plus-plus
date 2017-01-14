#ifndef SETI_STR_OBJECT_H
#define SETI_STR_OBJECT_H

#include <memory>
#include <iostream>

#include "run_time_error.h"
#include "ast/ast.h"
#include "interpreter/symbol-table.h"
#include "obj-type.h"
#include "func-object.h"
#include "slice-object.h"

namespace setti {
namespace internal {

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

  ObjectPtr ObjInt() override;

  ObjectPtr ObjReal() override;

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

  ObjectPtr Equal(ObjectPtr obj) override;

  ObjectPtr NotEqual(ObjectPtr obj) override;

  ObjectPtr Add(ObjectPtr obj) override;

  ObjectPtr Copy() override;

  ObjectPtr ObjCmd() override;

  ObjectPtr GetItem(ObjectPtr index) override;

  inline char Element(size_t i) {
    return value_[i];
  }

  ObjectPtr Element(const SliceObject& slice);

  std::shared_ptr<Object> Attr(std::shared_ptr<Object> self,
                               const std::string& name) override;

  std::string Print() override {
    return value_;
  }

 private:
  std::string value_;
};

class StringType: public TypeObject {
 public:
  StringType(ObjectPtr obj_type, SymbolTableStack&& sym_table);

  virtual ~StringType() {}

  ObjectPtr Constructor(Executor* /*parent*/,
                        std::vector<ObjectPtr>&& params) override;
};

class StringGetterFunc: public FuncObject {
 public:
  StringGetterFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);
};

class StringToLowerFunc: public FuncObject {
 public:
  StringToLowerFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);
};

class StringToUpperFunc: public FuncObject {
 public:
  StringToUpperFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);
};

class StringTrimmFunc: public FuncObject {
 public:
  StringTrimmFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);
};

class StringTrimmLeftFunc: public FuncObject {
 public:
  StringTrimmLeftFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);
};

class StringTrimmRightFunc: public FuncObject {
 public:
  StringTrimmRightFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);
};

class StringFindFunc: public FuncObject {
 public:
  StringFindFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);
};

class StringEndsWithFunc: public FuncObject {
 public:
  StringEndsWithFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);
};

}
}

#endif  // SETI_STR_OBJECT_H
