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

#ifndef SETI_ENV_SHELL_H
#define SETI_ENV_SHELL_H

#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "interpreter/symbol-table.h"
#include "ast/ast.h"
#include "interpreter/interpreter.h"

namespace seti {
namespace internal {

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

 private:
  EnvShell() = default;

  static EnvShell *instance_;

  pid_t shell_pgid_;
  struct termios shell_tmodes_;
  int shell_terminal_;
  int shell_is_interactive_;
};

}
}

#endif  // SETI_ENV_SHELL_H


