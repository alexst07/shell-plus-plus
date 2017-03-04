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

#ifndef SHPP_CMD_ENTRY_H
#define SHPP_CMD_ENTRY_H

#include <memory>

#include "interpreter/executor.h"
#include "interpreter/symbol-table.h"

namespace shpp {
namespace internal {

// command entry class
class CmdEntry {
 public:
  enum class Type {
    kDecl,
    kDef,
    kIn,
    kAlias
  };

  CmdEntry(Type type): type_(type) {}

  virtual void Exec(Executor* parent, std::vector<std::string>&& args) = 0;

  Type type() const noexcept {
    return type_;
  }

 private:
  Type type_;
};

using CmdEntryPtr = std::shared_ptr<CmdEntry>;

class CmdDeclEntry: public CmdEntry {
 public:
  CmdDeclEntry(std::shared_ptr<Block> start_node,
               const SymbolTableStack& symbol_table)
      : CmdEntry(Type::kDecl)
      , start_node_(start_node)
      , symbol_table_(symbol_table.MainTable()) {}

  void Exec(Executor* parent, std::vector<std::string>&& args) override;

 private:
  std::shared_ptr<Block> start_node_;
  SymbolTableStack symbol_table_;
};

class CmdInEntry: public CmdEntry {
 public:
  CmdInEntry(const SymbolTableStack& symbol_table)
      : CmdEntry(Type::kIn)
      , symbol_table_(symbol_table.MainTable()) {}

  virtual void Exec(Executor* parent, std::vector<std::string>&& args) = 0;

  void SetStdFd(int outfile, int errfile, int infile) {
    outfile_ = outfile;
    errfile_ = errfile;
    infile_ = infile;
  }

  std::tuple<int, int, int> GetStdFd() {
    return std::tuple<int, int, int>(outfile_, errfile_, infile_);
  }

  int GetStatus() {
    return status_;
  }

 protected:
  void SetStatus(int status) {
    status_ = status;
  }

 private:
  int outfile_;
  int errfile_;
  int infile_;
  int status_;
  SymbolTableStack symbol_table_;
};

template<class C>
void CmdSet(const std::string& name, SymbolTableStack& symbol_table) {
  CmdEntryPtr cmd_ptr(new C(symbol_table));
  symbol_table.SetCmd(name, cmd_ptr);
}

class CmdAliasEntry: public CmdEntry {
public:
 CmdAliasEntry(AstNode* start_node, const SymbolTableStack& symbol_table);
 const std::vector<std::string>& args() const noexcept;

private:
 std::vector<std::string> args_;
};

}
}

#endif  // SHPP_CMD_ENTRY_H


