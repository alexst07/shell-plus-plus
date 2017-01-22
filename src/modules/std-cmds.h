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

#ifndef SETI_STD_CMDS_H
#define SETI_STD_CMDS_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <tuple>

#include "cmd-entry.h"

namespace seti {
namespace internal {
namespace cmds {
namespace stdf {

class CdCmd: public CmdInEntry {
 public:
  CdCmd(const SymbolTableStack& symbol_table)
      : CmdInEntry(symbol_table) {}

  void Exec(Executor* /*parent*/, std::vector<std::string>&& args) override;
};

class ExitCmd: public CmdInEntry {
 public:
  ExitCmd(const SymbolTableStack& symbol_table)
      : CmdInEntry(symbol_table) {}

  void Exec(Executor* /*parent*/, std::vector<std::string>&& args) override;
};

inline void RegisterCmds(SymbolTableStack& sym_table) {
  CmdSet<CdCmd>("cd", sym_table);
  CmdSet<ExitCmd>("exit", sym_table);
}

}
}
}
}

#endif  // SETI_STD_CMDS_H


