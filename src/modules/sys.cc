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

#include "sys.h"

#include <cstdlib>

#include "utils/check.h"

namespace shpp {
namespace internal {
namespace module {
namespace sys {

SysModule::SysModule(ObjectPtr obj_type, SymbolTableStack&& sym_table)
    : Object(ObjectType::MODULE, obj_type, std::move(sym_table))
    , obj_factory_(symbol_table_stack()) {
  symbol_table_stack().NewTable();
}

std::shared_ptr<Object> SysModule::Attr(std::shared_ptr<Object>/*self*/,
    const std::string& name) {
  auto obj = symbol_table_stack().Lookup(name, false).Ref();
  return PassVar(obj, symbol_table_stack());
}

}
}
}
}
