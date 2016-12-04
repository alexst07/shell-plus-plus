#include "str-object.h"

#include <string>
#include <boost/variant.hpp>

#include "obj_type.h"
#include "object-factory.h"

namespace setti {
namespace internal {

ObjectPtr StringObject::Equal(ObjectPtr obj) {
  if (obj->type() != ObjectType::STRING) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type not supported"));
  }

  StringObject& obj_str = static_cast<StringObject&>(*obj);
  bool r = value_ == obj_str.value_;

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(r);
}

ObjectPtr StringObject::NotEqual(ObjectPtr obj) {
  if (obj->type() != ObjectType::STRING) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type not supported"));
  }

  StringObject& obj_str = static_cast<StringObject&>(*obj);
  bool r = value_ != obj_str.value_;

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(r);
}

ObjectPtr StringObject::Add(ObjectPtr obj) {
  if (obj->type() != ObjectType::STRING) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type not supported"));
  }

  StringObject& obj_str = static_cast<StringObject&>(*obj);
  std::string r = value_ + obj_str.value_;

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewString(r);
}

ObjectPtr StringObject::Copy() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewString(value_);
}

}
}
