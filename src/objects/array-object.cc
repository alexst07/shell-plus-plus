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

namespace shpp {
namespace internal {

ArrayIterObject::ArrayIterObject(ObjectPtr array_obj, ObjectPtr obj_type,
                                 SymbolTableStack&& sym_table)
    : BaseIter(ObjectType::ARRAY_ITER, obj_type, std::move(sym_table))
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

std::size_t ArrayObject::Hash() {
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

    std::tie(start, end, std::ignore) = SliceLogic(slice, value_.size());

    if (end > value_.size()) {
      throw RunTimeError(RunTimeError::ErrorCode::OUT_OF_RANGE,
                         boost::format("value of end of slice: %1% larger than "
                                       "the array size: %2%")
                         %end%value_.size());
    }

    value_.erase(value_.begin() + start, value_.begin() + end);
  } else {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("index must be int"));
  }
}

ObjectPtr ArrayObject::Equal(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());

  if (obj->type() != ObjectType::ARRAY) {
    return obj_factory.NewBool(false);
  }

  ArrayObject& array_obj = static_cast<ArrayObject&>(*obj);

  if (array_obj.value().size() != value_.size()) {
    return obj_factory.NewBool(false);
  }

  bool r = true;

  // Test each element on tuple
  for (size_t i = 0; i < value_.size(); i++) {
    ObjectPtr bool_obj = value_[i]->Equal(array_obj.value()[i]);
    r = r && (static_cast<BoolObject&>(*bool_obj).value());
  }

  return obj_factory.NewBool(r);
}

ObjectPtr ArrayObject::In(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());

  for (auto& item: value_) {
    ObjectPtr obj_cond = item->Equal(obj);
    if (obj_cond->type() != ObjectType::BOOL) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("equal method must return bool"));
    }

    bool v = static_cast<BoolObject&>(*obj_cond).value();

    if (v) {
      return obj_factory.NewBool(true);
    }
  }

  return obj_factory.NewBool(false);
}

bool ArrayObject::operator==(const Object& obj) {
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

void ArrayObject::SetItem(std::shared_ptr<Object> index,
    std::shared_ptr<Object> value) {
  if (index->type() != ObjectType::INT) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("index type not valid"));
  }

  int num_index = static_cast<IntObject&>(*index).value();

  if (num_index >= value_.size() ||  num_index < 0) {
    throw RunTimeError(RunTimeError::ErrorCode::OUT_OF_RANGE,
                       boost::format("value: %1% out of range of array")
                       %num_index);
  }

  value_[num_index] = value;
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

void ArrayObject::Insert(int index, ObjectPtr obj)
try {
  value_.insert (value_.begin() + index, obj);
} catch (std::out_of_range&) {
  throw RunTimeError(RunTimeError::ErrorCode::OUT_OF_RANGE,
                     boost::format("index out of range"));
} catch (std::bad_alloc&) {
  throw RunTimeError(RunTimeError::ErrorCode::BAD_ALLOC,
                     boost::format("bad alloc"));
}

ObjectPtr ArrayObject::Add(ObjectPtr obj) {
  SHPP_FUNC_CHECK_PARAM_TYPE(obj, array, ARRAY)

  std::vector<ObjectPtr> vec_obj;
  ArrayObject& array_ext = static_cast<ArrayObject&>(*obj);

  for (const auto& item: value_) {
    vec_obj.push_back(item);
  }

  for (const auto& item: array_ext.value()) {
    vec_obj.push_back(item);
  }

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewArray(std::move(vec_obj));
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
  RegisterMethod<ArrayExtendFunc>("extend", symbol_table_stack(), *this);
  RegisterMethod<ArrayInsertFunc>("insert", symbol_table_stack(), *this);
  RegisterMethod<ArrayRemoveFunc>("remove", symbol_table_stack(), *this);
  RegisterMethod<ArrayPopFunc>("pop", symbol_table_stack(), *this);
  RegisterMethod<ArrayClearFunc>("clear", symbol_table_stack(), *this);
  RegisterMethod<ArrayIndexFunc>("index", symbol_table_stack(), *this);
  RegisterMethod<ArrayCountFunc>("count", symbol_table_stack(), *this);
  RegisterMethod<ArraySortFunc>("sort", symbol_table_stack(), *this);
  RegisterMethod<ArrayReverseFunc>("reverse", symbol_table_stack(), *this);
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
  SHPP_FUNC_CHECK_NUM_PARAMS_UNTIL(params, 2, join)

  std::string delim = "";

  if (params.size() == 2) {
    SHPP_FUNC_CHECK_PARAM_TYPE(params[1], delim, STRING)
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
  SHPP_FUNC_CHECK_NUM_PARAMS_AT_LEAST(params, 2, append)

  ArrayObject& array_obj = static_cast<ArrayObject&>(*params[0]);

  for (size_t i = 1; i < params.size(); i++) {
    array_obj.Append(params[i]);
  }

  return params[0];
}

ObjectPtr ArrayExtendFunc::Call(Executor* /*parent*/,
                                std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS_AT_LEAST(params, 2, extend)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[1], array, ARRAY)

  ArrayObject& array_obj = static_cast<ArrayObject&>(*params[0]);
  ArrayObject& array_ext = static_cast<ArrayObject&>(*params[1]);

  for (const auto& item: array_ext.value()) {
    array_obj.Append(item);
  }

  return params[0];
}

ObjectPtr ArrayInsertFunc::Call(Executor* /*parent*/,
                                std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS_AT_LEAST(params, 3, insert)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[1], index, INT)

  ArrayObject& array_obj = static_cast<ArrayObject&>(*params[0]);
  int index = static_cast<IntObject&>(*params[1]).value();

  if (index < 0 || index > array_obj.value().size()) {
    throw RunTimeError(RunTimeError::ErrorCode::OUT_OF_RANGE,
                       boost::format("index out of range"));
  }

  array_obj.Insert(index, params[2]);

  return params[0];
}

ObjectPtr ArrayRemoveFunc::Call(Executor* /*parent*/,
                                std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS_AT_LEAST(params, 2, remove)

  ArrayObject& array_obj = static_cast<ArrayObject&>(*params[0]);
  std::vector<ObjectPtr>& vec = array_obj.value();

  vec.erase(std::remove_if(vec.begin(), vec.end(),
                           [&params](ObjectPtr& obj) {
                             return static_cast<BoolObject&>(
                               *obj->Equal(params[1])).value();
                           }),
    vec.end());

  return params[0];
}

ObjectPtr ArrayPopFunc::Call(Executor* /*parent*/,
                             std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS_AT_LEAST(params, 2, pop)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[1], index, INT)

  ArrayObject& array_obj = static_cast<ArrayObject&>(*params[0]);
  int index = static_cast<IntObject&>(*params[1]).value();

  if (index < 0 || index >= array_obj.value().size()) {
    throw RunTimeError(RunTimeError::ErrorCode::OUT_OF_RANGE,
                       boost::format("index out of range"));
  }

  ObjectPtr obj = array_obj.value()[index];

  array_obj.value().erase(array_obj.value().begin() + index);

  return obj;
}

ObjectPtr ArrayClearFunc::Call(Executor* /*parent*/,
                              std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS_AT_LEAST(params, 1, clear)

  ArrayObject& array_obj = static_cast<ArrayObject&>(*params[0]);

  array_obj.value().clear();

  return params[0];
}

ObjectPtr ArrayIndexFunc::Call(Executor* /*parent*/,
                                std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS_AT_LEAST(params, 2, index)

  ArrayObject& array_obj = static_cast<ArrayObject&>(*params[0]);

  ObjectFactory obj_factory(symbol_table_stack());

  int i = 0;
  for (auto& item: array_obj.value()) {
    ObjectPtr bool_obj = item->Equal(params[1]);
    if (static_cast<BoolObject&>(*bool_obj).value()) {
      return obj_factory.NewInt(i);
    }

    i++;
  }

  return obj_factory.NewBool(false);
}

ObjectPtr ArrayCountFunc::Call(Executor* /*parent*/,
                               std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 2, count)

  ArrayObject& array_obj = static_cast<ArrayObject&>(*params[0]);

  ObjectFactory obj_factory(symbol_table_stack());

  int i = 0;
  for (auto& item: array_obj.value()) {
    ObjectPtr bool_obj = item->Equal(params[1]);
    if (static_cast<BoolObject&>(*bool_obj).value()) {
      i++;
    }
  }

  return obj_factory.NewInt(i);
}

bool ArraySortFunc::Comp(ObjectPtr obj1, ObjectPtr obj2) {
  ObjectPtr obj_resp = obj1->Lesser(obj2);

  if (obj_resp->type() != Object::ObjectType::BOOL) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("operator less must return bool"));
  }

  return static_cast<BoolObject&>(*obj_resp).value();
}

bool ArraySortFunc::CompWithFunc(Executor* parent, ObjectPtr func,
                                 ObjectPtr obj1, ObjectPtr obj2) {
  std::vector<ObjectPtr> fparams = {obj1, obj2};

  ObjectPtr obj_resp = func->Call(parent, std::move(fparams));

  if (obj_resp->type() != Object::ObjectType::BOOL) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("operator less must return bool"));
  }

  return static_cast<BoolObject&>(*obj_resp).value();
}

ObjectPtr ArraySortFunc::Call(Executor* parent,
                              std::vector<ObjectPtr>&& params) {
  ArrayObject& array_obj = static_cast<ArrayObject&>(*params[0]);
  std::vector<ObjectPtr>& vec = array_obj.value();

  if (params.size() == 1) {
    using namespace std::placeholders;

    std::sort(vec.begin(), vec.end(),
        std::bind(&ArraySortFunc::Comp, this,_1, _2));

    return params[0];
  } else if (params.size() == 2) {
    using namespace std::placeholders;

    SHPP_FUNC_CHECK_PARAM_TYPE(params[1], comp, FUNC)

    std::sort(vec.begin(), vec.end(),
        std::bind(&ArraySortFunc::CompWithFunc, this, parent, params[1],
                  _1, _2));

    return params[0];
  }

  throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                     boost::format("num args incompatible"));
}

ObjectPtr ArrayReverseFunc::Call(Executor* parent,
                                 std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, reverse)

  ArrayObject& array_obj = static_cast<ArrayObject&>(*params[0]);
  std::vector<ObjectPtr>& vec = array_obj.value();

  std::reverse(vec.begin(), vec.end());

  return params[0];
}

ObjectPtr ArrayForEachFunc::Call(Executor* parent,
                                 std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 2, for_each)

  ArrayObject& array_obj = static_cast<ArrayObject&>(*params[0]);

  for (int i = 0; i < array_obj.Len(); i++) {
    std::vector<ObjectPtr> fparams(1, array_obj.Element(i));
    params[1]->Call(parent, std::move(fparams));
  }

  return params[0];
}

ObjectPtr ArrayMapFunc::Call(Executor* parent,
                             std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 2, map)

  ArrayObject& array_obj = static_cast<ArrayObject&>(*params[0]);

  for (int i = 0; i < array_obj.Len(); i++) {
    std::vector<ObjectPtr> fparams(1, array_obj.Element(i));
    array_obj.set(i, params[1]->Call(parent, std::move(fparams)));
  }

  return params[0];
}

}
}
