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

#include "str-object.h"

#include <string>
#include <boost/variant.hpp>
#include <boost/algorithm/string.hpp>

#include "obj-type.h"
#include "object-factory.h"
#include "utils/check.h"

namespace shpp {
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
  float v;

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

ObjectPtr StringObject::GetItem(ObjectPtr index) {
  if (index->type() == ObjectType::SLICE) {
    return Element(static_cast<SliceObject&>(*index));
  } else if (index->type() == ObjectType::INT) {
    char c = Element(static_cast<IntObject&>(*index).value());
    std::string str(1, c);
    ObjectFactory obj_factory(symbol_table_stack());
    return obj_factory.NewString(str);
  } else {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("index type not valid"));
  }
}

ObjectPtr StringObject::Element(const SliceObject& slice) {
  int start = 0;
  int end = value_.size();
  int step = 1;

  std::tie(start, end, step) = SliceLogic(slice, value_.size());

  std::string str = "";
  for (int i = start; i < end; i += step) {
    str += value_[i];
  }

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewString(str);
}

// string is always true, even when it is empty
// to test if the string is empty, a compare function
// must be used
ObjectPtr StringObject::ObjBool() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(true);
}

ObjectPtr StringObject::Not() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(false);
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
  ObjectPtr target_obj;

  if (obj->type() == ObjectType::STRING) {
    target_obj = obj;
  } else {
    target_obj = obj->ObjString();
  }

  if (target_obj->type() != ObjectType::STRING) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("string interface return no string"));
  }

  StringObject& obj_str = static_cast<StringObject&>(*target_obj);
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
  RegisterMethod<StringEndsWithFunc>("ends_with", symbol_table_stack(), *this);
  RegisterMethod<StringSplitFunc>("split", symbol_table_stack(), *this);
  RegisterMethod<StringFindFunc>("find", symbol_table_stack(), *this);
  RegisterMethod<StringReplaceFunc>("replace", symbol_table_stack(), *this);
  RegisterMethod<StringReplaceFunc>("replace_first", symbol_table_stack(),
                                    *this);
  RegisterMethod<StringReplaceFunc>("replace_last", symbol_table_stack(),
                                    *this);
  RegisterMethod<StringEraseAllFunc>("erase_all", symbol_table_stack(),
                                     *this);
  RegisterMethod<StringCountFunc>("count", symbol_table_stack(),
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
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, to_lower)

  StringObject& str_obj = static_cast<StringObject&>(*params[0]);
  std::string& str = const_cast<std::string&>(str_obj.value());
  boost::to_lower(str);

  return params[0];
}

ObjectPtr StringToUpperFunc::Call(Executor* /*parent*/,
                                 std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, to_upper)

  StringObject& str_obj = static_cast<StringObject&>(*params[0]);
  std::string& str = const_cast<std::string&>(str_obj.value());
  boost::to_upper(str);

  return params[0];
}

ObjectPtr StringTrimmFunc::Call(Executor* /*parent*/,
                                 std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, trim)

  StringObject& str_obj = static_cast<StringObject&>(*params[0]);
  std::string& str = const_cast<std::string&>(str_obj.value());
  boost::trim(str);

  return params[0];
}

ObjectPtr StringTrimmLeftFunc::Call(Executor* /*parent*/,
                                 std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, trim_left)

  StringObject& str_obj = static_cast<StringObject&>(*params[0]);
  std::string& str = const_cast<std::string&>(str_obj.value());
  boost::trim_left(str);

  return params[0];
}

ObjectPtr StringTrimmRightFunc::Call(Executor* /*parent*/,
                                 std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, trim_right)

  StringObject& str_obj = static_cast<StringObject&>(*params[0]);
  std::string& str = const_cast<std::string&>(str_obj.value());
  boost::trim_right(str);

  return params[0];
}

ObjectPtr StringFindFunc::Call(Executor* /*parent*/,
                               std::vector<ObjectPtr>&& params) {
  int pos = 0;

  if (params.size() == 3) {
    SHPP_FUNC_CHECK_PARAM_TYPE(params[2], pos, INT)

    pos = static_cast<IntObject&>(*params[2]).value();
  } else {
    SHPP_FUNC_CHECK_NUM_PARAMS(params, 2, find)
  }

  SHPP_FUNC_CHECK_PARAM_TYPE(params[1], str, STRING)

  StringObject& str_obj = static_cast<StringObject&>(*params[0]);
  std::string& self = const_cast<std::string&>(str_obj.value());

  std::string str = static_cast<StringObject&>(*params[1]).value();

  size_t f = self.find(str, pos);

  ObjectFactory obj_factory(symbol_table_stack());

  if (f == std::string::npos) {
    return obj_factory.NewBool(false);
  }

  return obj_factory.NewInt(f);
}

ObjectPtr StringCountFunc::Call(Executor* /*parent*/,
                                std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 2, find)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[1], str, STRING)

  StringObject& str_obj = static_cast<StringObject&>(*params[0]);
  std::string& self = const_cast<std::string&>(str_obj.value());
  const std::string& str = static_cast<StringObject&>(*params[1]).value();

  int count = 0;
  size_t npos = self.find(str, 0); // fist occurrence
  while(npos != std::string::npos) {
    count++;
    npos = self.find(str, npos+1);
  }

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewInt(count);
}

ObjectPtr StringEndsWithFunc::Call(Executor* /*parent*/,
                                   std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 2, ends_with)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[1], str, STRING)

  std::string str_obj = static_cast<StringObject&>(*params[0]).value();
  std::string str = static_cast<StringObject&>(*params[1]).value();

  ObjectFactory obj_factory(symbol_table_stack());

  if (str.length() > str_obj.length()) {
    return obj_factory.NewBool(false);
  }

  std::string sub_str = str_obj.substr(str_obj.length() - str.length());

  if (str == sub_str) {
    return obj_factory.NewBool(true);
  }

  return obj_factory.NewBool(false);
}

ObjectPtr StringSplitFunc::Call(Executor* /*parent*/,
                                std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 2, split)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[1], delim, STRING)

  StringObject& str_obj = static_cast<StringObject&>(*params[0]);
  std::string& str = const_cast<std::string&>(str_obj.value());
  const std::string& str_delim = static_cast<StringObject&>(*params[1]).value();

  std::vector<std::string> str_split;
  boost::algorithm::split(str_split, str, boost::is_any_of(str_delim),
                          boost::algorithm::token_compress_on);

  ObjectFactory obj_factory(symbol_table_stack());
  std::vector<ObjectPtr> obj_split;

  for (auto& s: str_split) {
    obj_split.push_back(obj_factory.NewString(s));
  }

  return obj_factory.NewArray(std::move(obj_split));
}

ObjectPtr StringReplaceFunc::Call(Executor* /*parent*/,
                                  std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 3, ends_with)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[1], str, STRING)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[2], str, STRING)

  StringObject& obj = static_cast<StringObject&>(*params[0]);
  StringObject& obj_find = static_cast<StringObject&>(*params[1]);
  StringObject& obj_replace = static_cast<StringObject&>(*params[2]);

  std::string& str = const_cast<std::string&>(obj.value());
  std::string& str_find = const_cast<std::string&>(obj_find.value());
  std::string& str_replace = const_cast<std::string&>(obj_replace.value());

  boost::replace_all(str, str_find, str_replace);

  return params[0];
}

ObjectPtr StringReplaceFirstFunc::Call(Executor* /*parent*/,
                                  std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 3, ends_with)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[1], str, STRING)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[2], str, STRING)

  StringObject& obj = static_cast<StringObject&>(*params[0]);
  StringObject& obj_find = static_cast<StringObject&>(*params[1]);
  StringObject& obj_replace = static_cast<StringObject&>(*params[2]);

  std::string& str = const_cast<std::string&>(obj.value());
  std::string& str_find = const_cast<std::string&>(obj_find.value());
  std::string& str_replace = const_cast<std::string&>(obj_replace.value());

  boost::replace_first(str, str_find, str_replace);

  return params[0];
}

ObjectPtr StringReplaceLastFunc::Call(Executor* /*parent*/,
                                  std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 3, ends_with)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[1], str, STRING)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[2], str, STRING)

  StringObject& obj = static_cast<StringObject&>(*params[0]);
  StringObject& obj_find = static_cast<StringObject&>(*params[1]);
  StringObject& obj_replace = static_cast<StringObject&>(*params[2]);

  std::string& str = const_cast<std::string&>(obj.value());
  std::string& str_find = const_cast<std::string&>(obj_find.value());
  std::string& str_replace = const_cast<std::string&>(obj_replace.value());

  boost::replace_last(str, str_find, str_replace);

  return params[0];
}

ObjectPtr StringEraseAllFunc::Call(Executor* /*parent*/,
                                  std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 2, ends_with)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[1], str, STRING)

  StringObject& obj = static_cast<StringObject&>(*params[0]);
  StringObject& obj_find = static_cast<StringObject&>(*params[1]);

  std::string& str = const_cast<std::string&>(obj.value());
  std::string& str_find = const_cast<std::string&>(obj_find.value());

  boost::erase_all(str, str_find);

  return params[0];
}

}
}
