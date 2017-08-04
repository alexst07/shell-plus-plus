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

#ifndef SHPP_FILE_OBJECT_H
#define SHPP_FILE_OBJECT_H

#include <memory>
#include <vector>
#include <fstream>
#include <string>

#include "run_time_error.h"
#include "ast/ast.h"
#include "interpreter/symbol-table.h"
#include "abstract-obj.h"
#include "slice-object.h"
#include "obj-type.h"
#include "func-object.h"

namespace shpp {
namespace internal {

class FileIterObject: public BaseIter {
 public:
  FileIterObject(ObjectPtr file_obj, ObjectPtr obj_type,
      SymbolTableStack&& sym_table);

  virtual ~FileIterObject() {}

  ObjectPtr Next() override;

  ObjectPtr HasNext() override;

  std::string Print() override {
    return std::string("[file_iter]");
  }

 private:
  ObjectPtr file_obj_;
  std::string line_;
};

class FileIterType: public TypeObject {
 public:
  FileIterType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("file_iter", obj_type, std::move(sym_table)) {}

  virtual ~FileIterType() {}

  ObjectPtr Constructor(Executor* /*parent*/,
                        Args&& params, KWArgs&&) override;
};

class FileObject: public Object {
 public:
  FileObject(const std::string& path, std::ios_base::openmode mode,
      ObjectPtr obj_type, SymbolTableStack&& sym_table);

  FileObject(const FileObject& obj) = delete;

  virtual ~FileObject() {}

  ObjectPtr ObjBool() override;

  ObjectPtr Not() override;

  ObjectPtr ObjIter(ObjectPtr obj) override;

  std::fstream& fs() {
   return fs_;
  }

  void Close();

  void Write(const std::string& str);

  std::string Read(int len);

  std::string ReadAll();

  int Size();

  int Tellg();

  void Seekg(std::streamoff off, std::ios_base::seekdir way);

  ObjectPtr Attr(std::shared_ptr<Object> self, const std::string& name);

 private:
  std::fstream fs_;
};

class FileType: public TypeObject {
 public:
  FileType(ObjectPtr obj_type, SymbolTableStack&& sym_table);

  virtual ~FileType() {}

  ObjectPtr Constructor(Executor*, Args&& params, KWArgs&&) override;
};

class FileCloseFunc: public FuncObject {
 public:
  FileCloseFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class FileReadLineFunc: public FuncObject {
 public:
  FileReadLineFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class FileReadFunc: public FuncObject {
 public:
  FileReadFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class FileReadAllFunc: public FuncObject {
 public:
  FileReadAllFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class FileSizeFunc: public FuncObject {
 public:
  FileSizeFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class FileWriteFunc: public FuncObject {
 public:
  FileWriteFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class FileTellgFunc: public FuncObject {
 public:
  FileTellgFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class FileSeekgFunc: public FuncObject {
 public:
  FileSeekgFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

}
}

#endif  // SHPP_FILE_OBJECT_H
