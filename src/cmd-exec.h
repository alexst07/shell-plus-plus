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

class Arguments {
 public:
  Arguments(std::vector<std::string>&& args);
  ~Arguments();

  char **argsv();

  std::vector<std::string> args();

  void Process();
 private:
  glob_t globbuf_;
  char **argv_;
  bool aloc_glob_;
};

class ProcessBase {
 public:
  ProcessBase(SymbolTableStack& sym_tab, std::vector<std::string>&& args,
      Executor* parent);

  virtual ~ProcessBase() = default;

  ProcessBase(const ProcessBase& p) = delete;

  ProcessBase& operator=(const ProcessBase& p) = delete;

  ProcessBase(ProcessBase&& p) = delete;

  ProcessBase& operator=(ProcessBase& p) = delete;

  ProcessBase& operator=(ProcessBase&& p) = delete;

  virtual void LaunchProcess(int infile, int outfile, int errfile, pid_t pgid,
                     bool foreground) = 0;

  virtual const std::vector<std::string>& Args() const;

  virtual std::vector<std::string>& Args();

  virtual pid_t pid() const;

  virtual ProcessBase& pid(pid_t v);

  virtual bool completed() const;

  virtual ProcessBase& completed(bool v);

  virtual bool stopped() const;

  virtual ProcessBase& stopped(bool v);

  virtual int status() const;

  virtual ProcessBase& status(int v);

 protected:
  inline SymbolTableStack& sym_tab() {
    return sym_tab_;
  }

  inline Executor* parent() {
    return parent_;
  }

 private:
  std::vector<std::string> args_;
  SymbolTableStack sym_tab_;
  pid_t pid_;
  bool completed_;
  bool stopped_;
  int status_;
  Executor* parent_;
};

class Process: public ProcessBase {
 public:
  Process(SymbolTableStack& sym_tab, std::vector<std::string>&& args,
      Executor* parent);

  ~Process() = default;

  void LaunchProcess(int infile, int outfile, int errfile, pid_t pgid,
                     bool foreground) override;

 private:
  void LaunchCmd(CmdEntryPtr cmd, std::vector<std::string>&& args);

  char** FillArgv(const std::vector<std::string>& args);
};

class ProcessSubShell: public ProcessBase {
 public:
  ProcessSubShell(SymbolTableStack& sym_tab, SubShell* sub_shell,
      Executor* parent);

  ~ProcessSubShell() = default;

  void LaunchProcess(int infile, int outfile, int errfile, pid_t pgid,
                     bool foreground) override;

 private:
  SubShell* sub_shell_;
};

class Job {
 public:
  Job(SymbolTableStack& sym_tab, Executor* parent, bool var_out_mode = false)
      : status_(0)
      , var_out_mode_(var_out_mode)
      , pgid_(0)
      , parent_(parent)
      , sym_tab_(sym_tab.MainTable()) {}

  int Status();
  void LaunchJob (int foreground);
  Job& Stdin(int stdin);
  Job& Stdout(int stdout);
  Job& Stderr(int stderr);
  Job& AddProcess(std::unique_ptr<ProcessBase>&& p);

 private:
  int MarkProcessStatus(pid_t pid, int status);
  bool JobIsStopped();
  bool JobIsCompleted();
  void WaitForJob();
  void PutJobInForeground(int cont);
  void PutJobInBackground(int cont);
  void LaunchInternalCmd(CmdEntryPtr cmd);
  // check if there was some error on command execution
  // and throw an exception is it was happens
  void CheckCmdError();

  std::vector<std::unique_ptr<ProcessBase>> process_;
  int stdin_, stdout_, stderr_;
  std::string strout_, strerr_;
  int status_;
  bool var_out_mode_;
  bool wait_;
  int shell_terminal_;
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
