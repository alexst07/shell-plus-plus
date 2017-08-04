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

#ifndef SHPP_ARRAY_OBJECT_H
#define SHPP_ARRAY_OBJECT_H

#include <memory>
#include <iostream>
#include <vector>

#include "run_time_error.h"
#include "ast/ast.h"
#include "interpreter/symbol-table.h"
#include "abstract-obj.h"
#include "slice-object.h"
#include "func-object.h"
#include "obj-type.h"

namespace shpp {
namespace internal {

class ArrayIterObject: public BaseIter {
 public:
  ArrayIterObject(ObjectPtr array_obj, ObjectPtr obj_type,
                  SymbolTableStack&& sym_table);

  virtual ~ArrayIterObject() {}

  ObjectPtr Equal(ObjectPtr obj) override;

  ObjectPtr Next() override;

  ObjectPtr HasNext() override;

  std::string Print() override {
    return std::string("[array_iter]");
  }

 private:
  // it uses the array object and position insted of c++ iterator
  // because the iterator object has need a shared_reference
  // of object, because the array could be removed from memory
  // if the object was created inside a loop for example
  // and the iterator could be used outside this loop
  ObjectPtr array_obj_;
  size_t pos_;
};

class ArrayObject: public Object {
 public:
   ArrayObject(std::vector<std::unique_ptr<Object>>&& value,
               ObjectPtr obj_type, SymbolTableStack&& sym_table);

   ArrayObject(std::vector<std::shared_ptr<Object>>&& value, ObjectPtr obj_type,
               SymbolTableStack&& sym_table);

   ArrayObject(const ArrayObject& obj);

   virtual ~ArrayObject() = default;

   inline Object* at(size_t i) {
     return value_.at(i).get();
   }

   inline std::shared_ptr<Object>& ElementRef(size_t i) {
     return value_.at(i);
   }

   inline std::shared_ptr<Object> Element(size_t i) {
     if (i >= value_.size()) {
       throw RunTimeError(RunTimeError::ErrorCode::OUT_OF_RANGE,
                          boost::format("index: %1% must be lower than "
                                        "the array size: %2%")%i%value_.size());
     }

     return value_.at(i);
   }

   ObjectPtr Element(const SliceObject& slice);

   ObjectPtr GetItem(ObjectPtr index) override;

   ObjectPtr ObjCmd() override;

   ObjectPtr Equal(ObjectPtr obj) override;

   ObjectPtr In(ObjectPtr obj) override;

   ObjectPtr Add(ObjectPtr obj) override;

   void DelItem(ObjectPtr index) override;

   ObjectPtr& GetItemRef(ObjectPtr index) override;

   void SetItem(std::shared_ptr<Object> index,
       std::shared_ptr<Object> value) override;

   std::shared_ptr<Object> Attr(std::shared_ptr<Object> self,
                                const std::string& name) override;

   void Append(ObjectPtr obj) {
     value_.push_back(obj);
   }

   void Insert(int index, ObjectPtr obj);

   inline void set(size_t i, std::unique_ptr<Object> obj) {
     Object* obj_ptr = obj.release();
     value_[i] = std::shared_ptr<Object>(obj_ptr);
   }

   inline void set(size_t i, std::shared_ptr<Object> obj) {
     value_[i] = obj;
   }

   ObjectPtr ObjIter(ObjectPtr obj) override;

   ObjectPtr ObjArray() override;

   std::size_t Hash() override;

   long int Len() override {
     return value_.size();
   }

   bool operator==(const Object& obj) override;

   size_t ArraySize() const noexcept {
     return value_.size();
   }

   std::vector<std::shared_ptr<Object>>& value() {
     return value_;
   }

   std::string Print() override;

 private:
  std::vector<std::shared_ptr<Object>> value_;
};

class ArrayType: public ContainerType {
 public:
  ArrayType(ObjectPtr obj_type, SymbolTableStack&& sym_table);

  virtual ObjectPtr Constructor(Executor* /*parent*/,
                                Args&& params, KWArgs&&);

  virtual ~ArrayType() {}
};

class ArrayJoinFunc: public FuncObject {
 public:
  ArrayJoinFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class ArrayAppendFunc: public FuncObject {
 public:
  ArrayAppendFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class ArrayExtendFunc: public FuncObject {
 public:
  ArrayExtendFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class ArrayInsertFunc: public FuncObject {
 public:
  ArrayInsertFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class ArrayRemoveFunc: public FuncObject {
 public:
  ArrayRemoveFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class ArrayPopFunc: public FuncObject {
 public:
  ArrayPopFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class ArrayClearFunc: public FuncObject {
 public:
  ArrayClearFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class ArrayIndexFunc: public FuncObject {
 public:
  ArrayIndexFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class ArrayCountFunc: public FuncObject {
 public:
  ArrayCountFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class ArraySortFunc: public FuncObject {
 public:
  ArraySortFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* parent, Args&& params, KWArgs&&);

 private:
  bool Comp(ObjectPtr obj1, ObjectPtr obj2);
  bool CompWithFunc(Executor* parent, ObjectPtr func, ObjectPtr obj1,
                    ObjectPtr obj2);
};

class ArrayReverseFunc: public FuncObject {
 public:
  ArrayReverseFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* parent, Args&& params, KWArgs&&);
};

class ArrayForEachFunc: public FuncObject {
 public:
  ArrayForEachFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* parent, Args&& params, KWArgs&&);
};

class ArrayMapFunc: public FuncObject {
 public:
  ArrayMapFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* parent, Args&& params, KWArgs&&);
};

}
}

#endif  // SHPP_ARRAY_OBJECT_H
