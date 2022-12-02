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

#include "glob-object.h"

#include "obj-type.h"
#include "object-factory.h"
#include "path.h"
#include "str-object.h"
#include "utils/check.h"
#include "utils/glob.h"

namespace shpp {
namespace internal {

GlobIterObject::GlobIterObject(ObjectPtr glob_obj, ObjectPtr obj_type,
                               SymbolTableStack&& sym_table)
    : BaseIter(ObjectType::CMD_ITER, obj_type, std::move(sym_table)),
      pos_(0),
      glob_obj_(glob_obj) {
  if (glob_obj->type() != ObjectType::GLOB) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("only glob_obj supported"));
  }
}

ObjectPtr GlobIterObject::Next() {
  GlobObject& glob_obj = static_cast<GlobObject&>(*glob_obj_);
  ObjectPtr item = glob_obj.GetGlobItem(pos_++);

  return item;
}

ObjectPtr GlobIterObject::HasNext() {
  ObjectFactory obj_factory(symbol_table_stack());
  GlobObject& glob_obj = static_cast<GlobObject&>(*glob_obj_);

  bool v = pos_ == glob_obj.Len();
  return obj_factory.NewBool(!v);
}

ObjectPtr GlobObject::ObjIter(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewGlobIter(obj);
}

GlobObject::GlobObject(const std::string& str_glob_expr, bool full,
                       ObjectPtr obj_type, SymbolTableStack&& sym_table)
    : Object(ObjectType::GLOB, obj_type, std::move(sym_table)),
      str_glob_expr_(str_glob_expr),
      full_(full),
      glob_result_vec_(
          full_ ? ExecGlob(str_glob_expr_, symbol_table_stack())
                : ExecGlobSimple(str_glob_expr_, symbol_table_stack())) {}

ObjectPtr GlobObject::ObjArray() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewArray(glob_result_vec_);
}

ObjectPtr GlobObject::In(ObjectPtr obj) {
  namespace fs = boost::filesystem;
  ObjectFactory obj_factory(symbol_table_stack());
  fs::path path;

  // verify if the obj passed is path, it's not, try to get the string object
  if (obj->type() == ObjectType::PATH) {
    path = static_cast<PathObject&>(*obj).value();
  } else {
    ObjectPtr obj_str = obj->ObjString();
    std::string str_path = static_cast<StringObject&>(*obj).value();
    path = fs::path(str_path);
  }

  // iterate over all elements of glob_result_vec_ and compare it as a boost
  // path
  for (ObjectPtr& it_obj : glob_result_vec_) {
    fs::path current_path = static_cast<PathObject&>(*it_obj).value();
    bool v = fs::equivalent(path, current_path);
    if (v) {
      return obj_factory.NewBool(true);
    }
  }

  return obj_factory.NewBool(false);
}

ObjectPtr GlobObject::ObjBool() {
  ObjectFactory obj_factory(symbol_table_stack());
  size_t len = Len();
  bool v = len > 0;
  return obj_factory.NewBool(v);
}

ObjectPtr GlobObject::Not() {
  ObjectFactory obj_factory(symbol_table_stack());
  size_t len = Len();
  bool v = len > 0;
  return obj_factory.NewBool(!v);
}

std::string GlobObject::Print() {
  ObjectFactory obj_factory(symbol_table_stack());
  ObjectPtr array_obj = obj_factory.NewArray(glob_result_vec_);
  return array_obj->Print();
}

std::shared_ptr<Object> GlobObject::Attr(std::shared_ptr<Object> self,
                                         const std::string& name) {
  ObjectPtr obj_type = ObjType();
  return static_cast<TypeObject&>(*obj_type).CallObject(name, self);
}

GlobType::GlobType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
    : TypeObject("glob", obj_type, std::move(sym_table)) {
  RegisterMethod<GlobSearchFunc>("search", symbol_table_stack(), *this);
}

ObjectPtr GlobType::Constructor(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 2, glob)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[0], expr, STRING)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[1], is_full, BOOL)

  StringObject& str_obj = static_cast<StringObject&>(*params[0]);
  const std::string& str_glob_expr = str_obj.value();
  bool is_full = static_cast<BoolObject&>(*params[1]).value();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewGlob(str_glob_expr, is_full);
}

ObjectPtr GlobSearchFunc::Call(Executor*, Args&& params, KWArgs&&) {
  namespace fs = boost::filesystem;
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 2, search)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[1], search, STRING)

  GlobObject& glob_obj = static_cast<GlobObject&>(*params[0]);
  StringObject& str_obj = static_cast<StringObject&>(*params[1]);

  std::vector<ObjectPtr>& paths = glob_obj.value();
  std::vector<ObjectPtr> found_paths = std::vector<ObjectPtr>();

  for (ObjectPtr& path_ojb : paths) {
    fs::path& path = static_cast<PathObject&>(*path_ojb).value();
    std::string str_path = path.string();
    size_t found = str_path.find(str_path);
    if (found != std::string::npos) {
      found_paths.push_back(path_ojb);
    }
  }

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewArray(found_paths);
}

}  // namespace internal
}  // namespace shpp