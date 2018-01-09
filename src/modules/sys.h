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

#ifndef SHPP_SYS_MODULE_H
#define SHPP_SYS_MODULE_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>

#include "objects/object-factory.h"

namespace shpp {
namespace internal {
namespace module {
namespace sys {

class SysModule: public Object {
 public:
  SysModule(ObjectPtr obj_type, SymbolTableStack&& sym_table);

  virtual ~SysModule() {}

  ObjectPtr Attr(std::shared_ptr<Object>/*self*/,
      const std::string& name) override;

  std::string Print() override {
    return std::string("[moule: sys]\n");
  }

 private:
  void InsertAttrFunc();
  ObjectPtr Argv();
  ObjectPtr Version();

  std::unordered_map<std::string, std::function<ObjectPtr()>> attrs_;
  ObjectFactory obj_factory_;
};


inline void RegisterModule(SymbolTableStack& sym_table) {
  // create a symbol table on the start
  SymbolTableStack table_stack;
  auto main_tab = sym_table.MainTable();
  table_stack.Push(main_tab, true);

  auto obj_type = sym_table.LookupSys("module").SharedAccess();
  ObjectPtr obj_module = ObjectPtr(new SysModule(obj_type,
      std::move(table_stack)));
  SymbolAttr symbol(obj_module, true);
  sym_table.InsertSysEntry("sys", std::move(symbol));
}

}
}
}
}

#endif  // SHPP_SYS_MODULE_H
