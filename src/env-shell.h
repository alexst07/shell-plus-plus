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

#ifndef SHPP_ENV_SHELL_H
#define SHPP_ENV_SHELL_H

#include <unordered_map>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "objects/abstract-obj.h"
#include "interpreter/symbol-table.h"
#include "ast/ast.h"
#include "interpreter/interpreter.h"

namespace shpp {
namespace internal {

#define SHPP_CMD_SIZE_MAX 256

struct CmdSharedError {
  int err_code;
  int except_code;
  bool error;
  char err_str[SHPP_CMD_SIZE_MAX];
};

class FileDescriptorMap {
 public:
  FileDescriptorMap();

  int& operator[](const std::string& name);

  int operator[](const std::string& name) const;

 private:
  std::unordered_map<std::string, int> map_;
};

class ImportTable {
 public:
  ImportTable() = default;
  ~ImportTable() = default;

  void AddModule(const std::string& name, ObjectPtr module);

  // if the module doesn't exists return ObjectPtr(nullptr)
  ObjectPtr GetModule(const std::string& name);

 private:
  std::unordered_map<std::string, ObjectPtr> module_table_;
};

class EnvShell {
 public:
  static EnvShell *instance() {
    if (!instance_) {
      instance_ = new EnvShell;
    }

    return instance_;
  }

  void InitShell();

  inline bool shell_is_interactive() const noexcept {
    return shell_is_interactive_;
  }

  inline struct termios* shell_tmodes() {
    return &shell_tmodes_;
  }

  inline int shell_terminal() {
    return shell_terminal_;
  }

  pid_t shell_pgid() {
    return shell_pgid_;
  }

  pid_t shell_pid() {
    return shell_pid_;
  }

  int shmid() const noexcept {
    return shmid_;
  }

  FileDescriptorMap& fd_map() {
    return fd_map_;
  }

  void interective_exec(bool v) {
    interective_exec_ = v;
  }

  bool interective_exec() {
    return interective_exec_;
  }

  ImportTable& GetImportTable() {
    return import_table_;
  }

  void SetArgv(std::vector<std::string>&& argv) {
    argv_ = std::move(argv);
  }

  const std::vector<std::string>& Argv() const {
    return argv_;
  }

  inline void last_background_pid(int pid) {
    last_background_pid_ = pid;
  }

  inline int last_background_pid() {
    return last_background_pid_;
  }

  inline void last_foreground_pid(int pid) {
    last_foreground_pid_ = pid;
  }

  inline int last_foreground_pid() {
    return last_foreground_pid_;
  }

  inline void last_foreground_exit_code(int code) {
    last_foreground_exit_code_ = code;
  }

  inline int last_foreground_exit_code() {
    return last_foreground_exit_code_;
  }

  ~EnvShell();

 private:
  EnvShell()
      : interective_exec_(false)
      , last_background_pid_(-1)
      , last_foreground_pid_(-1)
      , last_foreground_exit_code_(-1) {}

  static EnvShell *instance_;

  pid_t shell_pid_;
  pid_t shell_pgid_;
  FileDescriptorMap fd_map_;
  struct termios shell_tmodes_;
  int shell_terminal_;
  int shell_is_interactive_;
  int shmid_;
  bool interective_exec_;
  ImportTable import_table_;
  std::vector<std::string> argv_;
  int last_background_pid_;
  int last_foreground_pid_;
  int last_foreground_exit_code_;
};

}
}

#endif  // SHPP_ENV_SHELL_H
