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

#include "simple-object.h"

#include <string>
#include <boost/variant.hpp>

#include "obj-type.h"
#include "object-factory.h"

namespace seti {
namespace internal {

ObjectPtr NullObject::ObjBool() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(false);
}

ObjectPtr NullObject::Equal(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());

  if (obj->type() == ObjectType::NIL) {
    return obj_factory.NewBool(true);
  } else {
    return obj_factory.NewBool(false);
  }
}

ObjectPtr NullObject::NotEqual(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());

  if (obj->type() == ObjectType::NIL) {
    return obj_factory.NewBool(false);
  } else {
    return obj_factory.NewBool(true);
  }
}

ObjectPtr NullObject::And(ObjectPtr /*obj*/) {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(false);
}

ObjectPtr NullObject::Or(ObjectPtr obj) {
  return obj->ObjBool();
}

ObjectPtr BoolObject::ObjBool() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(value_);
}

ObjectPtr BoolObject::Equal(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());

  if (obj->type() == ObjectType::BOOL) {
    bool value = static_cast<const BoolObject&>(*obj).value_;
    return obj_factory.NewBool(value_ == value);
  } else {
    ObjectPtr obj_bool = obj->ObjBool();
    bool value = static_cast<const BoolObject&>(*obj_bool).value_;
    return obj_factory.NewBool(value_ == value);
  }
}

ObjectPtr BoolObject::NotEqual(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());

  if (obj->type() == ObjectType::BOOL) {
    bool value = static_cast<const BoolObject&>(*obj).value_;
    return obj_factory.NewBool(value_ != value);
  } else {
    ObjectPtr obj_bool = obj->ObjBool();
    bool value = static_cast<const BoolObject&>(*obj_bool).value_;
    return obj_factory.NewBool(value_ != value);
  }
}

ObjectPtr BoolObject::And(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());

  if (value_ == false) {
    return obj_factory.NewBool(false);
  }

  if (obj->type() == ObjectType::BOOL) {
    bool value = static_cast<const BoolObject&>(*obj).value_;
    ObjectFactory obj_factory(symbol_table_stack());
    return obj_factory.NewBool(value);
  } else {
    ObjectPtr obj_bool = obj->ObjBool();
    bool value = static_cast<const BoolObject&>(*obj_bool).value_;
    ObjectFactory obj_factory(symbol_table_stack());
    return obj_factory.NewBool(value);
  }
}

ObjectPtr BoolObject::Or(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());

  if (value_ == true) {
    return obj_factory.NewBool(true);
  }

  if (obj->type() == ObjectType::BOOL) {
    bool value = static_cast<const BoolObject&>(*obj).value_;
    ObjectFactory obj_factory(symbol_table_stack());
    return obj_factory.NewBool(value);
  } else {
    ObjectPtr obj_bool = obj->ObjBool();
    bool value = static_cast<const BoolObject&>(*obj_bool).value_;
    ObjectFactory obj_factory(symbol_table_stack());
    return obj_factory.NewBool(value);
  }
}

ObjectPtr BoolObject::Not() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(!value_);
}

////////////////////////////////////////////////////////////////////////////////
/// Int Object
////////////////////////////////////////////////////////////////////////////////

ObjectPtr IntObject::OperationObj(ObjectPtr obj, int op) {
  switch (obj->type()) {
    case ObjectType::INT: {
      IntObject& int_obj = static_cast<IntObject&>(*obj);
      int r = OperationArit(value_, int_obj.value_, op);
      ObjectFactory obj_factory(symbol_table_stack());
      return obj_factory.NewInt(r);
    } break;

    case ObjectType::REAL: {
      RealObject& real_obj = static_cast<RealObject&>(*obj);
      float r = OperationArit(value_, real_obj.value(), op);
      ObjectFactory obj_factory(symbol_table_stack());
      return obj_factory.NewReal(r);
    } break;

    default:
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("type not supported"));
  }
}

ObjectPtr IntObject::OperationObjInt(ObjectPtr obj, int op) {
  switch (obj->type()) {
    case ObjectType::INT: {
      IntObject& int_obj = static_cast<IntObject&>(*obj);
      int r = OperationArit(value_, int_obj.value_, op);
      ObjectFactory obj_factory(symbol_table_stack());
      return obj_factory.NewInt(r);
    } break;

    default:
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("type not supported"));
  }
}

ObjectPtr IntObject::ObjReal() {
  float v = static_cast<float>(value_);

  ObjectFactory obj_factory(symbol_table_stack());
  ObjectPtr obj_real(obj_factory.NewReal(v));

  return obj_real;
}

ObjectPtr IntObject::ObjString() {
  std::string v = std::to_string(value_);

  ObjectFactory obj_factory(symbol_table_stack());
  ObjectPtr obj_str(obj_factory.NewString(v));

  return obj_str;
}

ObjectPtr IntObject::OperationObjComp(ObjectPtr obj, int op) {
  switch (obj->type()) {
    case ObjectType::INT: {
      IntObject& int_obj = static_cast<IntObject&>(*obj);
      bool r = OperationComp(value_, int_obj.value_, op);
      ObjectFactory obj_factory(symbol_table_stack());
      return obj_factory.NewBool(r);
    } break;

    case ObjectType::REAL: {
      RealObject& real_obj = static_cast<RealObject&>(*obj);
      bool r = OperationComp(value_, real_obj.value(), op);
      ObjectFactory obj_factory(symbol_table_stack());
      return obj_factory.NewBool(r);
    } break;

    default: {
      ObjectFactory obj_factory(symbol_table_stack());
      return obj_factory.NewBool(false);
    }
  }
}

ObjectPtr IntObject::Copy() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewInt(value_);
}

ObjectPtr IntObject::Add(ObjectPtr obj) {
  return OperationObj(obj, 0);
}

ObjectPtr IntObject::Sub(ObjectPtr obj) {
  return OperationObj(obj, 1);
}

ObjectPtr IntObject::Mult(ObjectPtr obj) {
  return OperationObj(obj, 2);
}

ObjectPtr IntObject::Div(ObjectPtr obj) {
  switch (obj->type()) {
    case ObjectType::INT: {
      IntObject& int_obj = static_cast<IntObject&>(*obj);
      float b = int_obj.value();

      if (b == 0) {
        throw RunTimeError(RunTimeError::ErrorCode::ZERO_DIV,
                           boost::format("zero div indetermined"));
      }

      float r = static_cast<float>(value_)/ b;
      ObjectFactory obj_factory(symbol_table_stack());
      if ((value_% int_obj.value_) != 0) {
        return obj_factory.NewReal(r);
      } else {
        return obj_factory.NewInt(static_cast<int>(r));
      }
    } break;

    case ObjectType::REAL: {
      RealObject& real_obj = static_cast<RealObject&>(*obj);
      float b = real_obj.value();

      if (b == static_cast<float>(0)) {
        throw RunTimeError(RunTimeError::ErrorCode::ZERO_DIV,
                           boost::format("zero div indetermined"));
      }

      float r = static_cast<float>(value_)/ b;
      ObjectFactory obj_factory(symbol_table_stack());
      return obj_factory.NewReal(r);
    } break;

    default:
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("type not supported"));
  }
}

ObjectPtr IntObject::DivMod(ObjectPtr obj) {
  return OperationObjInt(obj, 4);
}

ObjectPtr IntObject::RightShift(ObjectPtr obj) {
  return OperationObjInt(obj, 5);
}

ObjectPtr IntObject::LeftShift(ObjectPtr obj) {
  return OperationObjInt(obj, 6);
}

ObjectPtr IntObject::Lesser(ObjectPtr obj) {
  return OperationObjComp(obj, 0);
}

ObjectPtr IntObject::Greater(ObjectPtr obj) {
  return OperationObjComp(obj, 1);
}

ObjectPtr IntObject::LessEqual(ObjectPtr obj) {
  return OperationObjComp(obj, 2);
}

ObjectPtr IntObject::GreatEqual(ObjectPtr obj) {
  return OperationObjComp(obj, 3);
}

ObjectPtr IntObject::Equal(ObjectPtr obj) {
  return OperationObjComp(obj, 4);
}

ObjectPtr IntObject::NotEqual(ObjectPtr obj) {
  return OperationObjComp(obj, 5);
}

ObjectPtr IntObject::BitAnd(ObjectPtr obj) {
  return OperationObjInt(obj, 7);
}

ObjectPtr IntObject::BitOr(ObjectPtr obj) {
  return OperationObjInt(obj, 8);
}

ObjectPtr IntObject::BitXor(ObjectPtr obj) {
  return OperationObjInt(obj, 9);
}

ObjectPtr IntObject::BitNot() {
  int r = ~value_;
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewInt(r);
}

ObjectPtr IntObject::UnaryAdd() {
  int r = +value_;
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewInt(r);
}

ObjectPtr IntObject::UnarySub() {
  int r = -value_;
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewInt(r);
}

////////////////////////////////////////////////////////////////////////////////
/// Real Object
////////////////////////////////////////////////////////////////////////////////
ObjectPtr RealObject::OperationObj(ObjectPtr obj, int op) {
  switch (obj->type()) {
    case ObjectType::INT: {
      IntObject& int_obj = static_cast<IntObject&>(*obj);
      float r = OperationArit(value_, static_cast<float>(int_obj.value()), op);
      ObjectFactory obj_factory(symbol_table_stack());
      return obj_factory.NewReal(r);
    } break;

    case ObjectType::REAL: {
      RealObject& real_obj = static_cast<RealObject&>(*obj);
      float r = OperationArit(value_, real_obj.value_, op);
      ObjectFactory obj_factory(symbol_table_stack());
      return obj_factory.NewReal(r);
    } break;

    default:
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("type not supported"));
  }
}

ObjectPtr RealObject::ObjInt() {
  int v = static_cast<int>(value_);

  ObjectFactory obj_factory(symbol_table_stack());
  ObjectPtr obj_int(obj_factory.NewReal(v));
  return obj_int;
}

ObjectPtr RealObject::ObjString() {
  std::string v = std::to_string(value_);

  ObjectFactory obj_factory(symbol_table_stack());
  ObjectPtr obj_str(obj_factory.NewString(v));

  return obj_str;
}

ObjectPtr RealObject::OperationObjComp(ObjectPtr obj, int op) {
  switch (obj->type()) {
    case ObjectType::INT: {
      IntObject& int_obj = static_cast<IntObject&>(*obj);
      bool r = OperationComp(value_, static_cast<float>(int_obj.value()), op);
      ObjectFactory obj_factory(symbol_table_stack());
      return obj_factory.NewBool(r);
    } break;

    case ObjectType::REAL: {
      RealObject& real_obj = static_cast<RealObject&>(*obj);
      bool r = OperationComp(value_, real_obj.value_, op);
      ObjectFactory obj_factory(symbol_table_stack());
      return obj_factory.NewBool(r);
    } break;

    default:
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("type not supported"));
  }
}

ObjectPtr RealObject::Copy() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewReal(value_);
}

ObjectPtr RealObject::Add(ObjectPtr obj) {
  return OperationObj(obj, 0);
}

ObjectPtr RealObject::Sub(ObjectPtr obj) {
  return OperationObj(obj, 1);
}

ObjectPtr RealObject::Mult(ObjectPtr obj) {
  return OperationObj(obj, 2);
}

ObjectPtr RealObject::Div(ObjectPtr obj) {
  return OperationObj(obj, 3);
}

ObjectPtr RealObject::Lesser(ObjectPtr obj) {
  return OperationObjComp(obj, 0);
}

ObjectPtr RealObject::Greater(ObjectPtr obj) {
  return OperationObjComp(obj, 1);
}

ObjectPtr RealObject::LessEqual(ObjectPtr obj) {
  return OperationObjComp(obj, 2);
}

ObjectPtr RealObject::GreatEqual(ObjectPtr obj) {
  return OperationObjComp(obj, 3);
}

ObjectPtr RealObject::Equal(ObjectPtr obj) {
  return OperationObjComp(obj, 4);
}

ObjectPtr RealObject::NotEqual(ObjectPtr obj) {
  return OperationObjComp(obj, 5);
}

ObjectPtr RealObject::UnaryAdd() {
  float r = +value_;
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewReal(r);
}

ObjectPtr RealObject::UnarySub() {
  float r = -value_;
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewReal(r);
}

}
}
