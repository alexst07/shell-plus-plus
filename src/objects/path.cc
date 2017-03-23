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

#include <unistd.h>
#include <sys/stat.h>

#include "obj-type.h"
#include "object-factory.h"
#include "utils/check.h"

namespace shpp {
namespace internal {

PathObject::PathObject(const std::string& str_path, ObjectPtr obj_type,
    SymbolTableStack&& sym_table)
    : Object(ObjectType::PATH, obj_type, std::move(sym_table))
    , path_{str_path} {}

PathObject::PathObject(const boost::filesystem::path& path, ObjectPtr obj_type,
    SymbolTableStack&& sym_table)
    : Object(ObjectType::PATH, obj_type, std::move(sym_table))
    , path_{path} {}

ObjectPtr PathObject::ObjString() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewString(path_.string());
}

ObjectPtr PathObject::ObjCmd() {
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

  SHPP_FUNC_CHECK_PARAM_TYPE(obj, equal, PATH)

  ObjectFactory obj_factory(symbol_table_stack());

  PathObject& path_obj = static_cast<PathObject&>(*obj);

  bool v = fs::equivalent(path_, path_obj.value());
  return obj_factory.NewBool(v);
}

PathType::PathType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
    : TypeObject("path", obj_type, std::move(sym_table)) {
  RegisterStaticMethod<PathPwdStaticFunc>("pwd",  symbol_table_stack(), *this);
  RegisterMethod<PathExistsFunc>("exits", symbol_table_stack(), *this);
  RegisterMethod<PathIsRegularFileFunc>("is_regular_file",
                                        symbol_table_stack(), *this);
  RegisterMethod<PathIsDirFunc>("is_dir", symbol_table_stack(), *this);
  RegisterMethod<PathIsSymLinkFunc>("is_sym_link", symbol_table_stack(), *this);
  RegisterMethod<PathIsReadableFunc>("is_readable",
                                     symbol_table_stack(), *this);
  RegisterMethod<PathIsWritableFunc>("is_writable",
                                     symbol_table_stack(), *this);
  RegisterMethod<PathIsExecutableFunc>("is_exec", symbol_table_stack(), *this);
  RegisterMethod<PathOwnerUidFunc>("uid_owner", symbol_table_stack(), *this);
  RegisterMethod<PathOwnerGidFunc>("gid_owner", symbol_table_stack(), *this);
  RegisterMethod<PathRootNameFunc>("root_name", symbol_table_stack(), *this);
  RegisterMethod<PathRootDirectoryFunc>("root_dir",
                                        symbol_table_stack(), *this);
  RegisterMethod<PathRootPathFunc>("root_path", symbol_table_stack(), *this);
  RegisterMethod<PathRelativePathFunc>("relative_path",
                                       symbol_table_stack(), *this);
  RegisterMethod<PathParentPathFunc>("parent_path",
                                     symbol_table_stack(), *this);
  RegisterMethod<PathFilenameFunc>("filename", symbol_table_stack(), *this);
  RegisterMethod<PathStemFunc>("stem", symbol_table_stack(), *this);
  RegisterMethod<PathExtensionFunc>("extension", symbol_table_stack(), *this);
  RegisterMethod<PathAbsoluteFunc>("absolute", symbol_table_stack(), *this);
}

ObjectPtr PathType::Constructor(Executor*, std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, path)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[0], path, STRING)

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
  SHPP_FUNC_CHECK_NO_PARAMS(params, pwd)

  namespace fs = boost::filesystem;

  fs::path path = fs::current_path();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewString(path.string());
}

ObjectPtr PathExistsFunc::Call(Executor* /*parent*/,
                               std::vector<ObjectPtr>&& params) {
  namespace fs = boost::filesystem;

  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, exists)
  fs::path& path = static_cast<PathObject&>(*params[0]).value();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(fs::exists(path));
}

ObjectPtr PathIsRegularFileFunc::Call(Executor* /*parent*/,
                               std::vector<ObjectPtr>&& params) {
  namespace fs = boost::filesystem;

  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, exists)
  fs::path& path = static_cast<PathObject&>(*params[0]).value();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(fs::is_regular_file(path));
}

ObjectPtr PathIsDirFunc::Call(Executor* /*parent*/,
                               std::vector<ObjectPtr>&& params) {
  namespace fs = boost::filesystem;

  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, exists)
  fs::path& path = static_cast<PathObject&>(*params[0]).value();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(fs::is_directory(path));
}

ObjectPtr PathIsSymLinkFunc::Call(Executor* /*parent*/,
                               std::vector<ObjectPtr>&& params) {
  namespace fs = boost::filesystem;

  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, exists)
  fs::path& path = static_cast<PathObject&>(*params[0]).value();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(fs::is_symlink(path));
}

ObjectPtr PathIsReadableFunc::Call(Executor* /*parent*/,
                               std::vector<ObjectPtr>&& params) {
  namespace fs = boost::filesystem;

  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, is_readable)
  fs::path& path = static_cast<PathObject&>(*params[0]).value();

  ObjectFactory obj_factory(symbol_table_stack());

  std::string str_path = path.string();
  if (access(str_path.c_str(), R_OK) < 0) {
    return obj_factory.NewBool(false);
  }

  return obj_factory.NewBool(true);
}

ObjectPtr PathIsWritableFunc::Call(Executor* /*parent*/,
                                   std::vector<ObjectPtr>&& params) {
  namespace fs = boost::filesystem;

  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, is_readable)
  fs::path& path = static_cast<PathObject&>(*params[0]).value();

  ObjectFactory obj_factory(symbol_table_stack());

  std::string str_path = path.string();
  if (access(str_path.c_str(), W_OK) < 0) {
   return obj_factory.NewBool(false);
  }

  return obj_factory.NewBool(true);
}

ObjectPtr PathIsExecutableFunc::Call(Executor* /*parent*/,
                                     std::vector<ObjectPtr>&& params) {
  namespace fs = boost::filesystem;

  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, is_exec)
  fs::path& path = static_cast<PathObject&>(*params[0]).value();

  ObjectFactory obj_factory(symbol_table_stack());

  std::string str_path = path.string();
  if (access(str_path.c_str(), X_OK) < 0) {
  return obj_factory.NewBool(false);
  }

  return obj_factory.NewBool(true);
}

ObjectPtr PathOwnerUidFunc::Call(Executor* /*parent*/,
                                 std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, is_executable)

  namespace fs = boost::filesystem;

  struct stat sb;

  fs::path& path = static_cast<PathObject&>(*params[0]).value();
  std::string str_path = path.string();

  if (stat(str_path.c_str(), &sb) == -1) {
  throw RunTimeError(RunTimeError::ErrorCode::FILE,
                     boost::format("%1%")%strerror(errno));
  }

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewInt(sb.st_uid);
}

ObjectPtr PathOwnerGidFunc::Call(Executor* /*parent*/,
                                 std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, is_executable)

  namespace fs = boost::filesystem;

  struct stat sb;

  fs::path& path = static_cast<PathObject&>(*params[0]).value();
  std::string str_path = path.string();

  if (stat(str_path.c_str(), &sb) == -1) {
  throw RunTimeError(RunTimeError::ErrorCode::FILE,
                     boost::format("%1%")%strerror(errno));
  }

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewInt(sb.st_gid);
}

ObjectPtr PathRootNameFunc::Call(Executor* /*parent*/,
                                 std::vector<ObjectPtr>&& params) {
  namespace fs = boost::filesystem;

  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, exists)
  fs::path& path = static_cast<PathObject&>(*params[0]).value();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewPath(path.root_name());
}

ObjectPtr PathRootDirectoryFunc::Call(Executor* /*parent*/,
                                      std::vector<ObjectPtr>&& params) {
  namespace fs = boost::filesystem;

  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, exists)
  fs::path& path = static_cast<PathObject&>(*params[0]).value();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewPath(path.root_directory());
}

ObjectPtr PathRootPathFunc::Call(Executor* /*parent*/,
                                 std::vector<ObjectPtr>&& params) {
  namespace fs = boost::filesystem;

  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, exists)
  fs::path& path = static_cast<PathObject&>(*params[0]).value();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewPath(path.root_path());
}

ObjectPtr PathRelativePathFunc::Call(Executor* /*parent*/,
                                     std::vector<ObjectPtr>&& params) {
  namespace fs = boost::filesystem;

  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, exists)
  fs::path& path = static_cast<PathObject&>(*params[0]).value();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewPath(path.relative_path());
}

ObjectPtr PathParentPathFunc::Call(Executor* /*parent*/,
                                   std::vector<ObjectPtr>&& params) {
  namespace fs = boost::filesystem;

  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, exists)
  fs::path& path = static_cast<PathObject&>(*params[0]).value();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewPath(path.parent_path());
}

ObjectPtr PathFilenameFunc::Call(Executor* /*parent*/,
                                 std::vector<ObjectPtr>&& params) {
  namespace fs = boost::filesystem;

  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, exists)
  fs::path& path = static_cast<PathObject&>(*params[0]).value();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewPath(path.filename());
}

ObjectPtr PathStemFunc::Call(Executor* /*parent*/,
                                 std::vector<ObjectPtr>&& params) {
  namespace fs = boost::filesystem;

  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, exists)
  fs::path& path = static_cast<PathObject&>(*params[0]).value();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewPath(path.stem());
}

ObjectPtr PathExtensionFunc::Call(Executor* /*parent*/,
                                 std::vector<ObjectPtr>&& params) {
  namespace fs = boost::filesystem;

  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, exists)
  fs::path& path = static_cast<PathObject&>(*params[0]).value();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewPath(path.extension());
}

ObjectPtr PathAbsoluteFunc::Call(Executor* /*parent*/,
                                 std::vector<ObjectPtr>&& params) {
  namespace fs = boost::filesystem;

  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, exists)
  fs::path& path = static_cast<PathObject&>(*params[0]).value();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewPath(canonical(path));
}

}
}
