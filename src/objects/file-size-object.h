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

#ifndef SHPP_FILE_SIZE_OBJECT_H
#define SHPP_FILE_SIZE_OBJECT_H

#include <memory>
#include <string>

#include "run_time_error.h"
#include "ast/ast.h"
#include "interpreter/symbol-table.h"
#include "abstract-obj.h"
#include "obj-type.h"
#include "func-object.h"

namespace shpp {
namespace internal {

class FileSizeObject: public Object {
 public:
  enum class Unit {
    BYTES = 0,
    KB = 1,
    MB = 2,
    GB = 3,
    TB = 4,
    AUTO = 5  // Automatic unit selection
  };

  FileSizeObject(long long bytes, Unit unit, ObjectPtr obj_type,
                 SymbolTableStack&& sym_table)
      : Object(ObjectType::CUSTON, obj_type, std::move(sym_table))
      , bytes_(bytes)
      , unit_(unit) {}

  FileSizeObject(const FileSizeObject& obj)
      : Object(obj), bytes_(obj.bytes_), unit_(obj.unit_) {}

  virtual ~FileSizeObject() {}

  FileSizeObject& operator=(const FileSizeObject& obj) {
    bytes_ = obj.bytes_;
    unit_ = obj.unit_;
    return *this;
  }

  inline long long bytes() const noexcept { return bytes_; }
  inline Unit unit() const noexcept { return unit_; }

  std::size_t Hash() override {
    std::hash<long long> ll_hash;
    return ll_hash(bytes_);
  }

  bool operator==(const Object& obj) override {
    if (obj.type() != ObjectType::CUSTON) {
      return false;
    }

    const FileSizeObject* fs_obj = dynamic_cast<const FileSizeObject*>(&obj);
    if (!fs_obj) {
      return false;
    }

    return bytes_ == fs_obj->bytes_;
  }

  ObjectPtr ObjBool() override;
  ObjectPtr ObjInt() override;
  ObjectPtr ObjReal() override;
  ObjectPtr ObjString() override;
  ObjectPtr Not() override;

  // Arithmetic operators
  ObjectPtr Add(ObjectPtr obj) override;
  ObjectPtr Sub(ObjectPtr obj) override;
  ObjectPtr Mult(ObjectPtr obj) override;
  ObjectPtr Div(ObjectPtr obj) override;

  // Comparison operators
  ObjectPtr Lesser(ObjectPtr obj) override;
  ObjectPtr Greater(ObjectPtr obj) override;
  ObjectPtr LessEqual(ObjectPtr obj) override;
  ObjectPtr GreatEqual(ObjectPtr obj) override;
  ObjectPtr Equal(ObjectPtr obj) override;
  ObjectPtr NotEqual(ObjectPtr obj) override;

  ObjectPtr Copy() override;

  std::string Print() override;

  ObjectPtr Attr(std::shared_ptr<Object> self,
                 const std::string& name) override;

 private:
  long long bytes_;
  Unit unit_;

  // Helper to determine the smaller unit between two file sizes
  static Unit GetSmallerUnit(Unit u1, Unit u2);
};

class FileSizeType: public TypeObject {
 public:
  FileSizeType(ObjectPtr obj_type, SymbolTableStack&& sym_table);

  virtual ~FileSizeType() {}

  ObjectPtr Constructor(Executor*, Args&& params, KWArgs&&) override;

  ObjectPtr Attr(std::shared_ptr<Object> self,
                 const std::string& name) override;
};

// Static methods for creating file sizes
class FileSizeBytesFunc: public FuncObject {
 public:
  FileSizeBytesFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class FileSizeKBFunc: public FuncObject {
 public:
  FileSizeKBFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class FileSizeMBFunc: public FuncObject {
 public:
  FileSizeMBFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class FileSizeGBFunc: public FuncObject {
 public:
  FileSizeGBFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class FileSizeTBFunc: public FuncObject {
 public:
  FileSizeTBFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

// Instance methods
class FileSizeToKBFunc: public FuncObject {
 public:
  FileSizeToKBFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class FileSizeToMBFunc: public FuncObject {
 public:
  FileSizeToMBFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class FileSizeToGBFunc: public FuncObject {
 public:
  FileSizeToGBFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class FileSizeToTBFunc: public FuncObject {
 public:
  FileSizeToTBFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class FileSizeToBytesFunc: public FuncObject {
 public:
  FileSizeToBytesFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

}  // namespace internal
}  // namespace shpp

#endif  // SHPP_FILE_SIZE_OBJECT_H

