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

#include "array-object.h"

#include <string>
#include <algorithm>
#include <boost/variant.hpp>

#include "obj-type.h"
#include "object-factory.h"
#include "simple-object.h"
#include "utils/check.h"

namespace seti {
namespace internal {

ArrayIterObject::ArrayIterObject(ObjectPtr array_obj, ObjectPtr obj_type,
                                 SymbolTableStack&& sym_table)
    : Object(ObjectType::ARRAY_ITER, obj_type, std::move(sym_table))
    , pos_(0) {
  if (array_obj->type() != ObjectType::ARRAY) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("invalid conversion to int"));
  }

  array_obj_ = array_obj;
}

ObjectPtr ArrayIterObject::Equal(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());

  if (obj->type() != ObjectType::ARRAY_ITER) {
    return obj_factory.NewBool(false);
  }

  ArrayIterObject& other = static_cast<ArrayIterObject&>(*obj);

  bool ptr_eq = obj.get() == array_obj_.get();
  bool pos_eq = other.pos_ == pos_;

  return obj_factory.NewBool(ptr_eq && pos_eq);
}

ObjectPtr ArrayIterObject::Next() {
  ArrayObject& array_obj = static_cast<ArrayObject&>(*array_obj_);

  if (pos_ >= array_obj.ArraySize()) {
    ObjectFactory obj_factory(symbol_table_stack());
    return obj_factory.NewNull();
  }

  return array_obj.Element(pos_++);
}

ObjectPtr ArrayIterObject::HasNext() {
  ObjectFactory obj_factory(symbol_table_stack());

  bool v = pos_ == static_cast<ArrayObject&>(*array_obj_).ArraySize();
  return obj_factory.NewBool(!v);
}

ArrayObject::ArrayObject(std::vector<std::unique_ptr<Object>>&& value,
                         ObjectPtr obj_type, SymbolTableStack&& sym_table)
   : Object(ObjectType::ARRAY, obj_type, std::move(sym_table))
   , value_(value.size()) {
  for (size_t i = 0; i < value.size(); i++) {
    Object* obj_ptr = value[i].release();
    value_[i] = std::shared_ptr<Object>(obj_ptr);
  }
}

ArrayObject::ArrayObject(std::vector<std::shared_ptr<Object>>&& value,
                         ObjectPtr obj_type, SymbolTableStack&& sym_table)
   : Object(ObjectType::ARRAY, obj_type, std::move(sym_table))
   , value_(value) {}

ArrayObject::ArrayObject(const ArrayObject& obj)
    : Object(obj), value_(obj.value_) {}

std::size_t ArrayObject::Hash() const {
  if (value_.empty()) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("hash of empty array is not valid"));
  }

  size_t hash = 0;

  // Executes xor operation with hash of each element of array
  for (auto& e: value_) {
    hash ^= e->Hash();
  }

  return hash;
}

ObjectPtr ArrayObject::ObjArray() {
  std::vector<ObjectPtr> to_vector;
  std::copy(value_.begin(), value_.end(),
      std::back_inserter(to_vector));

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewArray(std::move(to_vector));
}

ObjectPtr ArrayObject::ObjIter(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewArrayIter(obj);
}

ObjectPtr ArrayObject::ObjCmd() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewArray(value_);
}

void ArrayObject::DelItem(ObjectPtr index) {
  if (index->type() == ObjectType::INT) {
    // remove the item pointed by index
    int i = static_cast<IntObject&>(*index).value();
    value_.erase(value_.begin()+i);
  } else if (index->type() == ObjectType::SLICE) {
    // remove a range of items determined by the slice
    SliceObject& slice = static_cast<SliceObject&>(*index);

    int start = 0;
    int end = value_.size();
    int step = 1;

    std::tie(start, end, step) = SliceLogic(slice, value_.size());

    if (end > value_.size()) {
      throw RunTimeError(RunTimeError::ErrorCode::OUT_OF_RANGE,
                         boost::format("value of end of slice: %1% larger than "
                                       "the array size: %2%")
                         %end%value_.size());
    }

    for (int i = start; i < end; i += step) {
      value_.erase(value_.begin()+i);
    }
  } else {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("index must be int"));
  }
}

bool ArrayObject::operator==(const Object& obj) const {
  if (obj.type() != ObjectType::ARRAY) {
    return false;
  }

  const ArrayObject& array_obj = static_cast<const ArrayObject&>(obj);

  // If the tuples have different size, they are different
  if (array_obj.value_.size() != value_.size()) {
    return false;
  }

  bool r = true;

  // Test each element on tuple
  for (size_t i = 0; i < value_.size(); i++) {
    r = r && (array_obj.value_[i] == value_[i]);
  }

  return r;
}

ObjectPtr ArrayObject::Element(const SliceObject& slice) {
  int start = 0;
  int end = value_.size();
  int step = 1;

  std::tie(start, end, step) = SliceLogic(slice, value_.size());

  if (end > value_.size()) {
    throw RunTimeError(RunTimeError::ErrorCode::OUT_OF_RANGE,
                       boost::format("value of end of slice: %1% larger than "
                                     "the array size: %2%")%end%value_.size());
  }

  std::vector<std::shared_ptr<Object>> values;
  for (int i = start; i < end; i += step) {
    values.push_back(value_[i]);
  }

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewArray(std::move(values));
}

ObjectPtr ArrayObject::GetItem(ObjectPtr index) {
  if (index->type() == ObjectType::SLICE) {
    return Element(static_cast<SliceObject&>(*index));
  } else if (index->type() == ObjectType::INT) {
    return Element(static_cast<IntObject&>(*index).value());
  } else {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("index type not valid"));
  }
}

ObjectPtr& ArrayObject::GetItemRef(ObjectPtr index) {
  if (index->type() != ObjectType::INT) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("index type not valid"));
  }

  return ElementRef(static_cast<IntObject&>(*index).value());
}

std::string ArrayObject::Print() {
  std::string str;
  str = "[";

  for (const auto& e: value_) {
    str += e->Print();
    str += ", ";
  }

  str = str.substr(0, str.length() - 2);
  str += "]";

  return str;
}

std::shared_ptr<Object> ArrayObject::Attr(std::shared_ptr<Object> self,
                                          const std::string& name) {
  ObjectPtr obj_type = ObjType();
  return static_cast<TypeObject&>(*obj_type).CallObject(name, self);
}

ArrayType::ArrayType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
    : ContainerType("array", obj_type, std::move(sym_table)) {
  RegisterMethod<ArrayJoinFunc>("join", symbol_table_stack(), *this);
  RegisterMethod<ArrayAppendFunc>("append", symbol_table_stack(), *this);
  RegisterMethod<ArrayForEachFunc>("for_each", symbol_table_stack(), *this);
  RegisterMethod<ArrayMapFunc>("map", symbol_table_stack(), *this);
}

ObjectPtr ArrayType::Constructor(Executor* /*parent*/,
                                 std::vector<ObjectPtr>&& params) {
  if (params.size() != 1) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                       boost::format("%1%() takes exactly 1 argument")
                       %name());
  }

  return params[0]->ObjArray();
}

ObjectPtr ArrayJoinFunc::Call(Executor* /*parent*/,
                              std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS_UNTIL(params, 2, join)

  std::string delim = "";

  if (params.size() == 2) {
    SETI_FUNC_CHECK_PARAM_TYPE(params[1], delim, STRING)
    delim = static_cast<StringObject&>(*params[1]).value();
  }

  ArrayObject& array_obj = static_cast<ArrayObject&>(*params[0]);

  std::string result = "";
  for (size_t i = 0; i < array_obj.ArraySize(); i++) {
    if (array_obj.Element(i)->type() != ObjectType::STRING) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("element %1% type not string")%i);
    }

    result += static_cast<StringObject&>(*array_obj.Element(i)).value();
    result += delim;
  }

  result = result.substr(0, result.length()-delim.length());

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewString(result);
}

ObjectPtr ArrayAppendFunc::Call(Executor* /*parent*/,
                                std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS_AT_LEAST(params, 2, append)

  ArrayObject& array_obj = static_cast<ArrayObject&>(*params[0]);

  for (size_t i = 1; i < params.size(); i++) {
    array_obj.Append(params[i]);
  }

  return params[0];
}

ObjectPtr ArrayForEachFunc::Call(Executor* parent,
                                 std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 2, for_each)

  ArrayObject& array_obj = static_cast<ArrayObject&>(*params[0]);

  for (int i = 0; i < array_obj.Len(); i++) {
    std::vector<ObjectPtr> fparams(1, array_obj.Element(i));
    params[1]->Call(parent, std::move(fparams));
  }

  return params[0];
}

ObjectPtr ArrayMapFunc::Call(Executor* parent,
                                 std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 2, for_each)

  ArrayObject& array_obj = static_cast<ArrayObject&>(*params[0]);

  for (int i = 0; i < array_obj.Len(); i++) {
    std::vector<ObjectPtr> fparams(1, array_obj.Element(i));
    array_obj.set(i, params[1]->Call(parent, std::move(fparams)));
  }

  return params[0];
}

}
}
