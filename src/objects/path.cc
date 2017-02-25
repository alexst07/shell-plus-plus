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

#include "path.h"

#include "obj-type.h"
#include "object-factory.h"
#include "utils/check.h"

namespace seti {
namespace internal {

PathObject::PathObject(const std::string& str_path, ObjectPtr obj_type,
    SymbolTableStack&& sym_table)
    : Object(ObjectType::REGEX, obj_type, std::move(sym_table))
    , path_{str_path} {}

ObjectPtr PathObject::ObjString() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewString(path_.string());
}

boost::filesystem::path& PathObject::value() {
  return path_;
}

ObjectPtr PathObject::Attr(std::shared_ptr<Object> self,
                           const std::string& name) {
  ObjectPtr obj_type = ObjType();
  return static_cast<TypeObject&>(*obj_type).CallObject(name, self);
}

ObjectPtr PathObject::Equal(ObjectPtr obj) {
  namespace fs = boost::filesystem;

  SETI_FUNC_CHECK_PARAM_TYPE(obj, equal, PATH)

  ObjectFactory obj_factory(symbol_table_stack());

  PathObject& path_obj = static_cast<PathObject&>(*obj);

  bool v = fs::equivalent(path_, path_obj.value());
  return obj_factory.NewBool(v);
}

PathType::PathType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
    : TypeObject("path", obj_type, std::move(sym_table)) {
  RegisterMethod<PathExistsFunc>("exits", symbol_table_stack(), *this);
  RegisterStaticMethod<PathPwdStaticFunc>("pwd",  symbol_table_stack(), *this);
}

ObjectPtr PathType::Constructor(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1, path)
  SETI_FUNC_CHECK_PARAM_TYPE(params[0], path, STRING)

  StringObject& str_obj = static_cast<StringObject&>(*params[0]);
  const std::string& str_path = str_obj.value();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewPath(str_path);
}

ObjectPtr PathType::Attr(std::shared_ptr<Object> /*self*/,
                         const std::string& name) {
  return this->CallStaticObject(name);
}

ObjectPtr PathPwdStaticFunc::Call(Executor* /*parent*/,
                                  std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NO_PARAMS(params, pwd)

  namespace fs = boost::filesystem;

  fs::path path = fs::current_path();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewString(path.string());
}

ObjectPtr PathExistsFunc::Call(Executor* /*parent*/,
                               std::vector<ObjectPtr>&& params) {
  namespace fs = boost::filesystem;

  SETI_FUNC_CHECK_NUM_PARAMS(params, 1, exists)
  fs::path& path = static_cast<PathObject&>(*params[0]).value();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(fs::exists(path));
}

}
}
