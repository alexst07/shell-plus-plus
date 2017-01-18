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

#ifndef SETI_MAP_OBJECT_H
#define SETI_MAP_OBJECT_H

#include <memory>
#include <iostream>
#include <vector>

#include "run_time_error.h"
#include "ast/ast.h"
#include "interpreter/symbol-table.h"
#include "abstract-obj.h"

namespace seti {
namespace internal {

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

  ObjectPtr GetItem(ObjectPtr index) override;

  ObjectPtr& GetItemRef(ObjectPtr index) override;

  ObjectPtr ObjIter(ObjectPtr obj) override;

  // Return the reference for an object on the map, if there is no
  // entry for this index, create a new empty with this entry and
  // return its reference
  ObjectPtr& ElementRef(ObjectPtr obj_index);

  // Return a tuple object with the element and a bool object
  std::shared_ptr<Object> Element(ObjectPtr obj_index);

  // Create, this method doesn't do any kind of verification
  // the caller method must check if the entry exists on map or not
  ObjectPtr& Insert_(ObjectPtr obj_index);

  bool Exists(ObjectPtr obj_index);

  long int Len() override;

  const Map& value() const noexcept {
    return value_;
  }

  Map& value() noexcept {
    return value_;
  }

  std::string Print() override {
    std::string str = "{";

    for (auto& list: value_) {
      for (auto& pair: list.second) {
        str += "(";
        str += pair.first->Print();
        str += ", ";
        str += pair.second->Print();
        str += "), ";
      }
    }

    str = str.substr(0, str.length() - 3);
    str += "}";

    return str;
  }

 private:
   Map value_;
};

class MapIterObject: public Object {
 public:
  MapIterObject(ObjectPtr map_obj, ObjectPtr obj_type,
                SymbolTableStack&& sym_table);

  virtual ~MapIterObject() {}

  ObjectPtr Equal(ObjectPtr obj) override;

  ObjectPtr Next() override;

  ObjectPtr HasNext() override;

  std::string Print() override {
    return std::string("[map_iter]");
  }

 private:
  // it uses the array object and position insted of c++ iterator
  // because the iterator object has need a shared_reference
  // of object, because the array could be removed from memory
  // if the object was created inside a loop for example
  // and the iterator could be used outside this loop
  ObjectPtr map_obj_;
  std::unordered_map<size_t, std::vector<std::pair<ObjectPtr, ObjectPtr>>>
      ::iterator pos_;

  size_t pos_vec_;
};

}
}

#endif  // SETI_MAP_OBJECT_H
