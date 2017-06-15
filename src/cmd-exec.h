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

#ifndef SHPP_CMD_EXEC_H
#define SHPP_CMD_EXEC_H

#include <string>
#include <memory>
#include <stack>
#include <list>
#include <functional>
#include <boost/variant.hpp>
#include <termios.h>
#include <glob.h>

#include "objects/abstract-obj.h"
#include "interpreter/symbol-table.h"
#include "cmd-entry.h"
#include "env-shell.h"

namespace shpp {
namespace internal {

typedef std::vector<std::string> SimpleCmdData;

struct CmdIoData {
  using ObjectRef = std::reference_wrapper<ObjectPtr>;

  enum class Direction {
    IN,
    IN_VARIABLE,
    OUT,
    OUT_APPEND,
    OUT_VARIABLE
  };

  // it is handle as file or variable content depending
  // of direction option
  boost::variant<std::string, ObjectRef> content_;
  bool all_;
  int n_iface_;
  Direction in_out_;
};

typedef std::list<CmdIoData> CmdIoListData;

struct CmdIoRedirectData {
  CmdIoListData io_list_;
  SimpleCmdData cmd_;
};

class CmdPipeListData {
 public:
  CmdPipeListData() = default;
  ~CmdPipeListData() = default;

  void Push(CmdIoRedirectData&& cmd) {
    boost::variant<CmdIoRedirectData, std::string> v(std::move(cmd));
    pipe_list_.push_back(std::move(v));
  }

  void Push(std::string&& cmd) {
    boost::variant<CmdIoRedirectData, std::string> v(std::move(cmd));
    pipe_list_.push_back(std::move(v));
  }

 private:
  std::vector<boost::variant<CmdIoRedirectData, std::string>> pipe_list_;
};

class CmdOperationData {
 public:
  enum class Operation {
    AND,
    OR
  };

  CmdOperationData() = default;
  ~CmdOperationData() = default;

  void Push(CmdPipeListData&& cmd1, CmdPipeListData&& cmd2, Operation op) {
    cmd_.push(std::move(cmd1));
    cmd_.push(std::move(cmd2));
    op_.push(op);
  }

  std::stack<CmdPipeListData> cmd_;
  std::stack<Operation> op_;
};


typedef boost::variant<SimpleCmdData, CmdIoRedirectData,
    CmdPipeListData, CmdOperationData> CmdData;

struct CmdTable {
  CmdData cmd_;
  bool expr_;
};

class Arguments {
 public:
  Arguments(std::vector<std::string>&& args);
  ~Arguments();

  char **argsv();

  std::vector<std::string> args();

  void Process();
 private:
  glob_t globbuf_;
  std::vector<std::string>&& args_;
  char **argv_;
  bool aloc_glob_;
};

struct Process {
  Process(SymbolTableStack& sym_tab, std::vector<std::string>&& args);

  ~Process() = default;

  Process(const Process& p);

  Process& operator=(const Process& p);

  Process(Process&& p);

  Process& operator=(Process&& p);

  void LaunchProcess(int infile, int outfile, int errfile, pid_t pgid,
                     bool foreground);

  void LaunchCmd(CmdEntryPtr cmd, std::vector<std::string>&& args);

  char** FillArgv(const std::vector<std::string>& args);

  void CloseFileDescriptor(int fd);

  std::vector<std::string> args_;
  pid_t pid_;
  bool completed_;
  bool stopped_;
  int status_;
  Executor* parent_;
  SymbolTableStack sym_tab_;
};


struct Job {
  Job(SymbolTableStack& sym_tab, bool var_out_mode = false)
      : status_(0)
      , var_out_mode_(var_out_mode)
      , pgid_(0)
      , sym_tab_(sym_tab.MainTable()) {}

  void LaunchJob (int foreground);
  int MarkProcessStatus(pid_t pid, int status);
  bool JobIsStopped();
  bool JobIsCompleted();
  void WaitForJob();
  int Status();
  void PutJobInForeground(int cont);
  void PutJobInBackground(int cont);
  void LaunchInternalCmd(CmdEntryPtr cmd);

  // check if there was some error on command execution
  // and throw an exception is it was happens
  void CheckCmdError();

  std::vector<Process> process_;
  int stdin_, stdout_, stderr_;
  std::string strout_, strerr_;
  bool var_out_mode_;
  bool wait_;
  int status_;
  int shell_terminal_;
  int shell_is_interactive_;
  pid_t pgid_;
  int shell_pgid_;
  struct termios tmodes_;
  struct termios shell_tmodes_;
  Executor* parent_;
  SymbolTableStack sym_tab_;
};

}
}

#endif  // SHPP_CMD_EXEC_H
