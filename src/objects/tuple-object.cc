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

#include "tuple-object.h"

#include "simple-object.h"
#include "object-factory.h"

namespace shpp {
namespace internal {

std::size_t TupleObject::Hash() {
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

bool TupleObject::operator==(const Object& obj) {
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

ObjectPtr TupleObject::Element(const SliceObject& slice) {
  int start = 0;
  int end = value_.size();
  int step = 1;

  std::tie(start, end, step) = SliceLogic(slice, value_.size());

  std::vector<std::shared_ptr<Object>> values;
  for (int i = start; i < end; i += step) {
    values.push_back(value_[i]);
  }

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewArray(std::move(values));
}

ObjectPtr TupleObject::GetItem(ObjectPtr index) {
  if (index->type() == ObjectType::SLICE) {
    return Element(static_cast<SliceObject&>(*index));
  } else if (index->type() == ObjectType::INT) {
    return Element(static_cast<IntObject&>(*index).value());
  } else {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("index type not valid"));
  }
}

ObjectPtr& TupleObject::GetItemRef(ObjectPtr index) {
  if (index->type() != ObjectType::INT) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("index type not valid"));
  }

  return ElementRef(static_cast<IntObject&>(*index).value());
}

}
}
