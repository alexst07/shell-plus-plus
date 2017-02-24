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

#include "regex.h"

#include "obj-type.h"
#include "object-factory.h"
#include "utils/check.h"

namespace seti {
namespace internal {

RegexObject::RegexObject(const std::string& str_expr, ObjectPtr obj_type,
    SymbolTableStack&& sym_table)
try
    : Object(ObjectType::REGEX, obj_type, std::move(sym_table))
    , str_expr_(str_expr)
    , re_(str_expr_) {
} catch (std::regex_error& e) {
  throw RunTimeError(RunTimeError::ErrorCode::REGEX,
                     boost::format("%1%")% e.what());
}

ObjectPtr RegexObject::ObjString() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewString(str_expr_);
}

std::vector<std::vector<std::string>> RegexObject::Search(
    const std::string& str_search) {
  std::string s = str_search;
  std::smatch m;
  std::vector<std::vector<std::string>> vet_ret;

  while (std::regex_search(s, m, re_)) {
    std::vector<std::string> group;
    for (auto& x:m) {
      group.push_back(x);
    }

    vet_ret.push_back(std::move(group));
    s = m.suffix().str();
  }

  return vet_ret;
}

bool RegexObject::Match(const std::string& str) {
  return std::regex_match (str, re_);
}

ObjectPtr RegexObject::Attr(std::shared_ptr<Object> self,
                            const std::string& name) {
  ObjectPtr obj_type = ObjType();
  return static_cast<TypeObject&>(*obj_type).CallObject(name, self);
}

RegexType::RegexType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
    : TypeObject("regex", obj_type, std::move(sym_table)) {
  RegisterMethod<RegexMatchFunc>("match", symbol_table_stack(), *this);
  RegisterMethod<RegexSearchFunc>("search", symbol_table_stack(), *this);
}

ObjectPtr RegexType::Constructor(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1, regex)
  SETI_FUNC_CHECK_PARAM_TYPE(params[0], regex, STRING)

  const std::string& str = static_cast<StringObject&>(*params[0]).value();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewRegex(str);
}

ObjectPtr RegexMatchFunc::Call(Executor* /*parent*/,
                               std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 2, match)
  SETI_FUNC_CHECK_PARAM_TYPE(params[1], match, STRING)

  RegexObject& regex_obj = static_cast<RegexObject&>(*params[0]);
  const std::string& str = static_cast<StringObject&>(*params[1]).value();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(regex_obj.Match(str));
}

ObjectPtr RegexSearchFunc::Call(Executor* /*parent*/,
                                std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 2, search)
  SETI_FUNC_CHECK_PARAM_TYPE(params[0], search, REGEX)
  SETI_FUNC_CHECK_PARAM_TYPE(params[1], search, STRING)

  RegexObject& regex_obj = static_cast<RegexObject&>(*params[0]);
  const std::string& str = static_cast<StringObject&>(*params[1]).value();

  auto vec = regex_obj.Search(str);
  ObjectFactory obj_factory(symbol_table_stack());

  std::vector<ObjectPtr> vec_obj;

  for (auto& row: vec) {
    std::vector<ObjectPtr> row_obj;
    for (auto& elem: row) {
      ObjectPtr obj_str = obj_factory.NewString(elem);
      row_obj.push_back(obj_str);
    }

    ObjectPtr row_vec_obj = obj_factory.NewArray(std::move(row_obj));
    vec_obj.push_back(row_vec_obj);
  }

  return obj_factory.NewArray(std::move(vec_obj));
}

}
}
