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

namespace seti {
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

  long int Len() override {
    return value_.size();
  }

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

class StringSplitFunc: public FuncObject {
 public:
  StringSplitFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);
};

}
}

#endif  // SETI_STR_OBJECT_H
