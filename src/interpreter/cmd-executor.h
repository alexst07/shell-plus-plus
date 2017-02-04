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

#ifndef SETI_CMD_EXECUTOR_H
#define SETI_CMD_EXECUTOR_H

#include <tuple>

#include "executor.h"
#include "cmd-exec.h"

namespace seti {
namespace internal {

typedef std::tuple<int, std::string, std::string> CmdExprData;

class CmdExecutor: public Executor {
 public:
  CmdExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  CmdExprData ExecGetResult(CmdFull *node);

  CmdExprData ExecCmdGetResult(Cmd *node);

  CmdExprData ExecCmdBinOp(CmdAndOr* cmd);

  int ExecCmdBinOp(CmdAndOr* cmd, bool wait);

  int Exec(CmdFull *node);

  int ExecCmd(Cmd *node, bool wait);

  int ExecSimpleCmd(SimpleCmd *node, bool wait);

  CmdExprData ExecSimpleCmdWithResult(SimpleCmd *node);

  std::string CmdOutput() const;

 private:
  std::string cmd_output_;
};

class SimpleCmdExecutor: public Executor {
 public:
  SimpleCmdExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  std::vector<std::string> Exec(SimpleCmd *node);
};

class CmdIoRedirectListExecutor: public Executor {
 public:
  CmdIoRedirectListExecutor(Executor* parent,
                            SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  int Exec(CmdIoRedirectList *node, bool wait);

  CmdExprData Exec(CmdIoRedirectList *node);

  int GetInteger(Literal* integer);

  static std::string FileName(Executor *parent, FilePathCmd* file_path,
                              bool trim = true);

  void PrepareData(Job &job, CmdIoRedirectList *node);

  int Var2Pipe(std::string var);

  int Str2Pipe(const std::string& str);
};

class CmdPipeSequenceExecutor: public Executor {
 public:
  CmdPipeSequenceExecutor(Executor* parent,
                            SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  int Exec(CmdPipeSequence *node, bool wait);
  CmdExprData Exec(CmdPipeSequence *node);
  void PopulateCmd(Job& job, CmdPipeSequence *node);
  void AddCommand(Job& job, Cmd *cmd);
};

// functions to manipulate command
std::string ResolveCmdExpr(Executor *parent, CmdValueExpr *cmd_expr);

// execute an expression for array types or simple string type
std::vector<std::string> ResolveFullTypeCmdExpr(Executor* parent,
                                                CmdValueExpr* cmd_expr);

std::string ExtractCmdExprFromString(Executor* parent, const std::string& str);

// functions to manipulate file
int CreateFile(std::string file_name);
int AppendFile(std::string file_name);
int ReadFile(std::string file_name);

}
}

#endif  // SETI_CMD_EXECUTOR_H
