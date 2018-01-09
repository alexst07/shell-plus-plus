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

#ifndef SHPP_INTERPRETER_H
#define SHPP_INTERPRETER_H

#include <functional>
#include <fstream>

#include "symbol-table.h"

namespace shpp {
namespace internal {

class Executor;

class ScriptStream {
 public:
  ScriptStream(const std::string& filename)
      : filename_(filename), fs_(filename) {}

  ScriptStream(const std::string& filename, std::ifstream&& fs)
      : filename_(filename), fs_(std::move(fs)) {}

  std::ifstream& fs() { return fs_; }
  const std::string& filename() const { return filename_; }
  bool IsOpen() const { return fs_.is_open(); }
 private:
  std::string filename_;
  std::ifstream fs_;
};

class Interpreter {
 public:
  Interpreter(bool main = false);

  ~Interpreter() = default;

  inline SymbolTableStack& SymTableStack() {
    return symbol_table_stack_;
  }

  void Exec(ScriptStream& file, std::vector<std::string>&& args = {});
  void ExecInterative(const std::function<std::string(Executor *, bool)> &func);

  std::shared_ptr<Object> LookupSymbol(const std::string& name);

  void RegisterMainModule(const std::string& full_path);

 private:
  void RegisterVars();
  void RegisterArgs(std::vector<std::string>&& args);
  void InsertVar(const std::string& name, std::shared_ptr<Object> obj);
  void RegisterFileVars(const std::string& file);
  void RegisterSysVars();
  void ShowErrors(RunTimeError& e, const std::string& code,
      const std::string& filename);

  SymbolTablePtr symbol_table_;
  SymbolTableStack symbol_table_stack_;
  SymbolTableStack sys_symbol_table_stack_;
  std::unique_ptr<StatementList> stmt_list_;
  bool main_;
  std::string full_path_;
};

std::vector<std::string> SplitFileLines(const std::string str_file);

}
}

#endif  // SHPP_INTERPRETER_H
