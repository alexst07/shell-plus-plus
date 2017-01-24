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

#ifndef SETI_ENV_FUNCS_H
#define SETI_STD_FUNCS_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <tuple>

#include "objects/object-factory.h"

namespace seti {
namespace internal {
namespace module {
namespace env {

class SetFunc: public FuncObject {
 public:
  SetFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);

 private:
  ObjectFactory obj_factory_;
};

class GetFunc: public FuncObject {
 public:
  GetFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
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

class AppendFunc: public FuncObject {
 public:
  AppendFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);

 private:
  ObjectFactory obj_factory_;
};

class UnsetFunc: public FuncObject {
 public:
  UnsetFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);

 private:
  ObjectFactory obj_factory_;
};

inline void RegisterModule(SymbolTableStack& sym_table) {
  ModuleCustonObject::MemberTable table = {
    {"set",                 ObjectMethod<SetFunc>(sym_table)},
    {"get",                 ObjectMethod<GetFunc>(sym_table)},
    {"append",              ObjectMethod<AppendFunc>(sym_table)},
    {"exists",              ObjectMethod<ExistsFunc>(sym_table)},
    {"unset",               ObjectMethod<UnsetFunc>(sym_table)}
  };

  ObjectFactory obj_factory(sym_table);
  ObjectPtr obj_module = obj_factory.NewModule("env", std::move(table));
  SymbolAttr symbol(obj_module, true);
  sym_table.InsertEntry("env", std::move(symbol));
}

}
}
}
}

#endif  // SETI_STD_FUNCS_H


