#include "int-object.h"

#include <string>
#include <boost/variant.hpp>

#include "obj_type.h"
#include "object-factory.h"

namespace setti {
namespace internal {

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
      float r = OperationArit(value_, real_obj.value(), op);
      ObjectFactory obj_factory(symbol_table_stack());
      return obj_factory.NewReal(r);
    } break;

    default:
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("type not supported"));
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
      float r = static_cast<float>(value_)/ int_obj.value_;
      ObjectFactory obj_factory(symbol_table_stack());
      if ((value_% int_obj.value_) != 0) {
        return obj_factory.NewReal(r);
      } else {
        return obj_factory.NewInt(static_cast<int>(r));
      }
    } break;

    case ObjectType::REAL: {
      RealObject& real_obj = static_cast<RealObject&>(*obj);
      float r = static_cast<float>(value_)/ real_obj.value();
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

}
}
