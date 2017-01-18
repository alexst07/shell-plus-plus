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

#ifndef SETI_PATH_FUNCS_H
#define SETI_PATH_FUNCS_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <tuple>

#include "objects/object-factory.h"

namespace seti {
namespace internal {
namespace module {
namespace path {

class PwdFunc: public FuncObject {
 public:
  PwdFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);

 private:
  ObjectFactory obj_factory_;
};

class ExistsFunc: public FuncObject {
 public:
  ExistsFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);

 private:
  ObjectFactory obj_factory_;
};

class IsRegularFile: public FuncObject {
 public:
  IsRegularFile(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);

 private:
  ObjectFactory obj_factory_;
};

class IsDirFunc: public FuncObject {
 public:
  IsDirFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);

 private:
  ObjectFactory obj_factory_;
};

class IsSymLink: public FuncObject {
 public:
  IsSymLink(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);

 private:
  ObjectFactory obj_factory_;
};

class IsReadable: public FuncObject {
 public:
  IsReadable(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);

 private:
  ObjectFactory obj_factory_;
};

class IsWritable: public FuncObject {
 public:
  IsWritable(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);

 private:
  ObjectFactory obj_factory_;
};

class IsExecutable: public FuncObject {
 public:
  IsExecutable(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);

 private:
  ObjectFactory obj_factory_;
};

class OwnerUid: public FuncObject {
 public:
  OwnerUid(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);

 private:
  ObjectFactory obj_factory_;
};

class OwnerGid: public FuncObject {
 public:
  OwnerGid(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);

 private:
  ObjectFactory obj_factory_;
};


inline void RegisterModule(SymbolTableStack& sym_table) {
  ModuleCustonObject::MemberTable table = {
    {"pwd",                 ObjectMethod<PwdFunc>(sym_table)},
    {"exists",              ObjectMethod<ExistsFunc>(sym_table)},
    {"is_dir",              ObjectMethod<IsDirFunc>(sym_table)},
    {"is_regular_file",     ObjectMethod<IsRegularFile>(sym_table)},
    {"is_symlink",          ObjectMethod<IsSymLink>(sym_table)},
    {"is_readable",         ObjectMethod<IsReadable>(sym_table)},
    {"is_writable",         ObjectMethod<IsWritable>(sym_table)},
    {"is_executable",       ObjectMethod<IsExecutable>(sym_table)},
    {"owner_uid",           ObjectMethod<OwnerUid>(sym_table)},
    {"owner_gid",           ObjectMethod<OwnerGid>(sym_table)}
  };

  ObjectFactory obj_factory(sym_table);
  ObjectPtr obj_module = obj_factory.NewModule("path", std::move(table));
  SymbolAttr symbol(obj_module, true);
  sym_table.InsertEntry("path", std::move(symbol));
}

}
}
}
}

#endif  // SETI_STD_FUNCS_H


