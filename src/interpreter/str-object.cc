#include "str-object.h"

#include <string>
#include <boost/variant.hpp>
#include <boost/algorithm/string.hpp>

#include "obj_type.h"
#include "object-factory.h"
#include "utils/check.h"

namespace setti {
namespace internal {

ObjectPtr StringObject::ObjInt() {
  int v;

  try {
    v = std::stoi(value_);
  } catch (std::exception&) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("invalid string to int"));
  }

  ObjectFactory obj_factory(symbol_table_stack());
  ObjectPtr obj_int(obj_factory.NewInt(v));
  return obj_int;
}

ObjectPtr StringObject::ObjReal() {
  int v;

  try {
    v = std::stof(value_);
  } catch (std::exception&) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("invalid string to real"));
  }

  ObjectFactory obj_factory(symbol_table_stack());
  ObjectPtr obj_real(obj_factory.NewReal(v));

  return obj_real;
}

ObjectPtr StringObject::Equal(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());

  if (obj->type() != ObjectType::STRING) {
    return obj_factory.NewBool(false);
  }

  StringObject& obj_str = static_cast<StringObject&>(*obj);
  bool r = value_ == obj_str.value_;

  return obj_factory.NewBool(r);
}

ObjectPtr StringObject::NotEqual(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());

  if (obj->type() != ObjectType::STRING) {
    return obj_factory.NewBool(true);
  }

  StringObject& obj_str = static_cast<StringObject&>(*obj);
  bool r = value_ != obj_str.value_;

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

ObjectPtr StringObject::ObjCmd() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewString(value_);
}

std::shared_ptr<Object> StringObject::Attr(std::shared_ptr<Object> self,
                                           const std::string& name) {
  ObjectPtr obj_type = ObjType();
  return static_cast<TypeObject&>(*obj_type).CallObject(name, self);
}

StringType::StringType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
    : TypeObject("string", obj_type, std::move(sym_table)) {
  RegisterMethod<StringGetterFunc>("at", symbol_table_stack(), *this);
  RegisterMethod<StringToLowerFunc>("to_lower", symbol_table_stack(), *this);
  RegisterMethod<StringToUpperFunc>("to_upper", symbol_table_stack(), *this);
  RegisterMethod<StringTrimmFunc>("trim", symbol_table_stack(), *this);
  RegisterMethod<StringTrimmLeftFunc>("trim_left", symbol_table_stack(), *this);
  RegisterMethod<StringTrimmRightFunc>("trim_right", symbol_table_stack(),
                                       *this);
}

ObjectPtr StringType::Constructor(Executor* /*parent*/,
                                  std::vector<ObjectPtr>&& params) {
  if (params.size() != 1) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                       boost::format("real() takes exactly 1 argument"));
  }

  if (params[0]->type() == ObjectType::STRING) {
    ObjectFactory obj_factory(symbol_table_stack());

    ObjectPtr obj_str(obj_factory.NewString(static_cast<StringObject&>(
        *params[0]).value()));

    return obj_str;
  }

  return params[0]->ObjString();
}

ObjectPtr StringGetterFunc::Call(Executor* /*parent*/,
                                 std::vector<ObjectPtr>&& params) {
  StringObject& str_obj = static_cast<StringObject&>(*params[0]);
  IntObject& int_obj = static_cast<IntObject&>(*params[1]);

  char c = str_obj.value()[int_obj.value()];
  ObjectFactory obj_factory(symbol_table_stack());
  std::string cstr(&c, 1);
  return obj_factory.NewString(cstr);
}

ObjectPtr StringToLowerFunc::Call(Executor* /*parent*/,
                                 std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1, to_lower)

  StringObject& str_obj = static_cast<StringObject&>(*params[0]);
  std::string& str = const_cast<std::string&>(str_obj.value());
  boost::to_lower(str);

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewNull();
}

ObjectPtr StringToUpperFunc::Call(Executor* /*parent*/,
                                 std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1, to_upper)

  StringObject& str_obj = static_cast<StringObject&>(*params[0]);
  std::string& str = const_cast<std::string&>(str_obj.value());
  boost::to_upper(str);

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewNull();
}

ObjectPtr StringTrimmFunc::Call(Executor* /*parent*/,
                                 std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1, trim)

  StringObject& str_obj = static_cast<StringObject&>(*params[0]);
  std::string& str = const_cast<std::string&>(str_obj.value());
  boost::trim(str);

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewNull();
}

ObjectPtr StringTrimmLeftFunc::Call(Executor* /*parent*/,
                                 std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1, trim_left)

  StringObject& str_obj = static_cast<StringObject&>(*params[0]);
  std::string& str = const_cast<std::string&>(str_obj.value());
  boost::trim_left(str);

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewNull();
}

ObjectPtr StringTrimmRightFunc::Call(Executor* /*parent*/,
                                 std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1, trim_right)

  StringObject& str_obj = static_cast<StringObject&>(*params[0]);
  std::string& str = const_cast<std::string&>(str_obj.value());
  boost::trim_right(str);

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewNull();
}
}
}
