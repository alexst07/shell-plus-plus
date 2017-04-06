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

#ifndef SHPP_TUPLE_OBJECT_H
#define SHPP_TUPLE_OBJECT_H

#include <memory>
#include <iostream>
#include <vector>

#include "run_time_error.h"
#include "ast/ast.h"
#include "interpreter/symbol-table.h"
#include "abstract-obj.h"
#include "slice-object.h"

namespace shpp {
namespace internal {

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

   ObjectPtr Element(const SliceObject& slice);

   ObjectPtr GetItem(ObjectPtr index) override;

   ObjectPtr& GetItemRef(ObjectPtr index) override;

   ObjectPtr ObjArray() override;

   std::size_t Hash() override;

   bool operator==(const Object& obj) override;

   long int Len() override {
     return value_.size();
   }

   std::string Print() override {
     std::string str = "(";
     for (const auto& e: value_) {
       str += e->Print();
       str += ", ";
     }

     str = str.substr(0, str.length() - 2);
     str += ")";

     return str;
   }

 private:
  std::vector<std::shared_ptr<Object>> value_;
};

}
}

#endif  // SHPP_TUPLE_OBJECT_H
