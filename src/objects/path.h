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

#ifndef SHPP_PATH_OBJECT_H
#define SHPP_PATH_OBJECT_H

#include <memory>
#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>

#include "run_time_error.h"
#include "ast/ast.h"
#include "interpreter/symbol-table.h"
#include "abstract-obj.h"
#include "obj-type.h"
#include "func-object.h"

namespace shpp {
namespace internal {

class PathObject: public Object {
 public:
   PathObject(const std::string& str_path, ObjectPtr obj_type,
      SymbolTableStack&& sym_table);

  PathObject(const boost::filesystem::path& path, ObjectPtr obj_type,
      SymbolTableStack&& sym_table);

   virtual ~PathObject() {}

   boost::filesystem::path& value();

   ObjectPtr ObjString() override;

   ObjectPtr ObjCmd() override;

   ObjectPtr Attr(std::shared_ptr<Object> self,
                  const std::string& name) override;

   ObjectPtr Equal(ObjectPtr obj) override;

   std::string Print() override {
     return "path<" + path_.string() + ">";
   }

   long int Len() override {
     return path_.string().size();
   }

 private:
  boost::filesystem::path path_;
};

class PathType: public TypeObject {
 public:
  PathType(ObjectPtr obj_type, SymbolTableStack&& sym_table);

  virtual ~PathType() {}

  ObjectPtr Attr(std::shared_ptr<Object>, const std::string& name) override;

  ObjectPtr Constructor(Executor*, Args&& params, KWArgs&&) override;
};

class PathPwdStaticFunc: public FuncObject {
 public:
  PathPwdStaticFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class PathExistsFunc: public FuncObject {
 public:
  PathExistsFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class PathIsRegularFileFunc: public FuncObject {
 public:
  PathIsRegularFileFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class PathIsDirFunc: public FuncObject {
 public:
  PathIsDirFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class PathIsSymLinkFunc: public FuncObject {
 public:
  PathIsSymLinkFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class PathIsReadableFunc: public FuncObject {
 public:
  PathIsReadableFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class PathIsWritableFunc: public FuncObject {
 public:
  PathIsWritableFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class PathIsExecutableFunc: public FuncObject {
 public:
  PathIsExecutableFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class PathOwnerUidFunc: public FuncObject {
 public:
  PathOwnerUidFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class PathOwnerGidFunc: public FuncObject {
 public:
  PathOwnerGidFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class PathRootNameFunc: public FuncObject {
 public:
  PathRootNameFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class PathRootDirectoryFunc: public FuncObject {
 public:
  PathRootDirectoryFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class PathRootPathFunc: public FuncObject {
 public:
  PathRootPathFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class PathRelativePathFunc: public FuncObject {
 public:
  PathRelativePathFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class PathParentPathFunc: public FuncObject {
 public:
  PathParentPathFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class PathFilenameFunc: public FuncObject {
 public:
  PathFilenameFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class PathStemFunc: public FuncObject {
 public:
  PathStemFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class PathExtensionFunc: public FuncObject {
 public:
  PathExtensionFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class PathAbsoluteFunc: public FuncObject {
 public:
  PathAbsoluteFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

}
}

#endif  // SHPP_PATH_OBJECT_H
