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

#include "env-shell.h"

#include <boost/locale.hpp>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/mman.h>

namespace shpp {
namespace internal {

FileDescriptorMap::FileDescriptorMap() {
  map_["0"] =  STDIN_FILENO;
  map_["1"] =  STDOUT_FILENO;
  map_["2"] =  STDERR_FILENO;
}

int& FileDescriptorMap::operator[](const std::string& name) {
  try {
    return map_.at(name);
  } catch (std::out_of_range& e) {
    throw RunTimeError(RunTimeError::ErrorCode::FD_NOT_FOUND,
                       boost::format("file descriptor '%1%' not found")
                       %name);
  }
}

int FileDescriptorMap::operator[](const std::string& name) const {
  try {
    return map_.at(name);
  } catch (std::out_of_range& e) {
    throw RunTimeError(RunTimeError::ErrorCode::FD_NOT_FOUND,
                       boost::format("file descriptor '%1%' not found")
                       %name);
  }
}

void ImportTable::AddModule(const std::string& name, ObjectPtr module) {
  module_table_.insert(std::pair<std::string, ObjectPtr>(name, module));
}

ObjectPtr ImportTable::GetModule(const std::string& name) {
  auto it = module_table_.find(name);

  if (it != module_table_.end()) {
    return it->second;
  }

  return ObjectPtr(nullptr);
}

EnvShell *EnvShell::instance_ = 0;

void EnvShell::InitShell() {
  // see if we are running interactively
  shell_terminal_ = STDIN_FILENO;
  shell_is_interactive_ = isatty(shell_terminal_);

  // starts the shared memory region
  shmid_ = shmget(IPC_PRIVATE, sizeof(CmdSharedError), 0640|IPC_CREAT);

  // gets pid of shell main process
  shell_pid_ = getpid();

  // initialize global boost locale
  boost::locale::generator gen;
  auto loc = gen("");
  std::locale::global(loc);
  std::cout.imbue(loc);

  if (shell_is_interactive_) {
    // loop until we are in the foreground
    while (tcgetpgrp(shell_terminal_) != (shell_pgid_ = getpgrp())) {
      kill(- shell_pgid_, SIGTTIN);
    }

    // ignore interactive and job-control signals
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

    // put ourselves in our own process group
    shell_pgid_ = getpid ();
    if (setpgid(shell_pgid_, shell_pgid_) < 0) {
      perror("Couldn't put the shell in its own process group");
      exit(1);
    }

    /* Grab control of the terminal.  */
    tcsetpgrp(shell_terminal_, shell_pgid_);

    /* Save default terminal attributes for shell.  */
    tcgetattr(shell_terminal_, &shell_tmodes_);
  }
}

EnvShell::~EnvShell() {
  shmctl(shmid_, IPC_RMID, 0);
}

}
}
