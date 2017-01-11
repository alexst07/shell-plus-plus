#include "array-object.h"

#include <string>
#include <boost/variant.hpp>

#include "obj-type.h"
#include "object-factory.h"
#include "simple-object.h"

namespace setti {
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

ObjectPtr ArrayObject::ObjIter(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewArrayIter(obj);
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
  if (index->type() == ObjectType::INT) {
    return ElementRef(static_cast<IntObject&>(*index).value());
  }
}

}
}
