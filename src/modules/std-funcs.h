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

#ifndef SHPP_STD_FUNCS_H
#define SHPP_STD_FUNCS_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <tuple>

#include "objects/object-factory.h"

namespace shpp {
namespace internal {
namespace module {
namespace stdf {

class PrintFunc: public FuncObject {
 public:
  PrintFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);

 private:
  ObjectFactory obj_factory_;
};

class PrintErrFunc: public FuncObject {
 public:
  PrintErrFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);

 private:
  ObjectFactory obj_factory_;
};

class ReadFunc: public FuncObject {
 public:
  ReadFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);

 private:
  ObjectFactory obj_factory_;
};

class LenFunc: public FuncObject {
 public:
  LenFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);

 private:
  ObjectFactory obj_factory_;
};

class GetLastForegroundPidFunc: public FuncObject {
 public:
  GetLastForegroundPidFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);

 private:
  ObjectFactory obj_factory_;
};

class GetLastBackgroundPidFunc: public FuncObject {
 public:
  GetLastBackgroundPidFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);

 private:
  ObjectFactory obj_factory_;
};

class GetLastForegroundExitFunc: public FuncObject {
 public:
  GetLastForegroundExitFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);

 private:
  ObjectFactory obj_factory_;
};


class CompFunc: public FuncObject {
 public:
  CompFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*pare'nt*/, Args&& params, KWArgs&&);

 private:
  ObjectFactory obj_factory_;
};

class RangeFunc: public FuncObject {
 public:
  RangeFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);

 private:
  ObjectFactory obj_factory_;
};

class AssertFunc: public FuncObject {
 public:
  AssertFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);

 private:
  ObjectFactory obj_factory_;
};

class ArgvFunc: public FuncObject {
 public:
  ArgvFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor*, Args&&, KWArgs&&);

 private:
  ObjectFactory obj_factory_;
};

class GetAttrObjFunc: public FuncObject {
 public:
  GetAttrObjFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor*, Args&&, KWArgs&&);

 private:
  ObjectFactory obj_factory_;
};

class GetAttrTypeFunc: public FuncObject {
 public:
  GetAttrTypeFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor*, Args&&, KWArgs&&);

 private:
  ObjectFactory obj_factory_;
};

class IsInteractiveFunc: public FuncObject {
 public:
  IsInteractiveFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);

 private:
  ObjectFactory obj_factory_;
};

class GlobFunc: public FuncObject {
 public:
  GlobFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);

 private:
  ObjectFactory obj_factory_;
};

class GlobRFunc: public FuncObject {
 public:
  GlobRFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);

 private:
  ObjectFactory obj_factory_;
};

class EvalFunc: public SpecialFuncObject {
 public:
  EvalFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : SpecialFuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr SpecialCall(Executor* parent, Args&& params, KWArgs&&,
      SymbolTableStack& curret_sym_tab) override;

 private:
  ObjectFactory obj_factory_;
};

class DumpSymbolTableFunc: public SpecialFuncObject {
 public:
  DumpSymbolTableFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : SpecialFuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr SpecialCall(Executor* parent, Args&& params, KWArgs&&,
      SymbolTableStack& curret_sym_tab) override;

 private:
  ObjectFactory obj_factory_;
};

class InstanceOfFunc: public FuncObject {
 public:
  InstanceOfFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* parent, Args&& params, KWArgs&&) override;

 private:
  ObjectFactory obj_factory_;
};

inline void RegisterModule(SymbolTableStack& sym_table) {
  ModuleCustonObject::MemberTable table = {
    {"print",                 ObjectMethod<PrintFunc>(sym_table)},
    {"print_err",             ObjectMethod<PrintErrFunc>(sym_table)},
    {"read",                  ObjectMethod<ReadFunc>(sym_table)},
    {"len",                   ObjectMethod<LenFunc>(sym_table)},
    {"range",                 ObjectMethod<RangeFunc>(sym_table)},
    {"comp",                  ObjectMethod<CompFunc>(sym_table)},
    {"assert",                ObjectMethod<AssertFunc>(sym_table)},
    {"argv",                  ObjectMethod<ArgvFunc>(sym_table)},
    {"get_attr_obj",          ObjectMethod<GetAttrObjFunc>(sym_table)},
    {"get_attr_type",         ObjectMethod<GetAttrTypeFunc>(sym_table)},
    {"is_interactive",        ObjectMethod<IsInteractiveFunc>(sym_table)},
    {"glob",                  ObjectMethod<GlobFunc>(sym_table)},
    {"globr",                 ObjectMethod<GlobRFunc>(sym_table)},
    {"instance_of",           ObjectMethod<InstanceOfFunc>(sym_table)},
    {"dump_symbol_table",     ObjectMethod<DumpSymbolTableFunc>(sym_table)},
    {"eval",                  ObjectMethod<EvalFunc>(sym_table)},
    {"get_last_foreground_pid",
          ObjectMethod<GetLastForegroundPidFunc>(sym_table)},
    {"get_last_background_pid",
          ObjectMethod<GetLastBackgroundPidFunc>(sym_table)},
    {"get_last_foreground_exit",
          ObjectMethod<GetLastForegroundExitFunc>(sym_table)}
  };

  for (auto& pair: table) {
    SymbolAttr sym_entry(pair.second, true);
    sym_table.InsertEntry(pair.first, std::move(sym_entry));
  }
}

}
}
}
}

#endif  // SHPP_STD_FUNCS_H
