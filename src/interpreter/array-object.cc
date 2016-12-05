#include "array-object.h"

#include <string>
#include <boost/variant.hpp>

#include "obj_type.h"
#include "object-factory.h"

namespace setti {
namespace internal {

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


}
}
