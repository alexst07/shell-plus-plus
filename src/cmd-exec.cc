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

#include "cmd-exec.h"

#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/shm.h>

#include "interpreter/cmd-executor.h"

namespace seti {
namespace internal {

Process::Process(SymbolTableStack& sym_tab, std::vector<std::string>&& args)
    : args_(std::move(args)), sym_tab_(sym_tab.MainTable())
    , completed_(false)
    , stopped_(false) {}

Process::Process(const Process& p): sym_tab_(p.sym_tab_) {
  args_ = p.args_;
  pid_ = p.pid_;
  completed_ = p.completed_;
  stopped_ = p.stopped_;
  status_ = p.status_;
}

Process& Process::operator=(const Process& p) {
  args_ = p.args_;
  pid_ = p.pid_;
  completed_ = p.completed_;
  stopped_ = p.stopped_;
  status_ = p.status_;
  sym_tab_ = p.sym_tab_;

  return *this;
}

Process::Process(Process&& p) {
  args_ = std::move(p.args_);
  pid_ = p.pid_;
  completed_ = p.completed_;
  stopped_ = p.stopped_;
  status_ = p.status_;
  sym_tab_ = std::move(p.sym_tab_);
}

Process& Process::operator=(Process&& p) {
  args_ = std::move(p.args_);
  pid_ = p.pid_;
  completed_ = p.completed_;
  stopped_ = p.stopped_;
  status_ = p.status_;
  sym_tab_ = std::move(p.sym_tab_);

  return *this;
}

void Process::LaunchProcess(int infile, int outfile, int errfile, pid_t pgid,
                            bool foreground) {
  pid_t pid;
  bool shell_is_interactive = EnvShell::instance()->shell_is_interactive();
  int shell_terminal = EnvShell::instance()->shell_terminal();

  if (shell_is_interactive) {
    // Put the process into the process group and give the process group
    // the terminal, if appropriate.
    // This has to be done both by the shell and in the individual
    // child processes because of potential race conditions.
    pid = getpid ();

    if (pgid == 0) {
      pgid = pid;
    }

    setpgid (pid, pgid);

    if (foreground) {
      // this part fix the problem of stuck on tcsetpgrp
      // sources: https://dev.haiku-os.org/ticket/3417
      // https://dev.haiku-os.org/attachment/ticket/3417/tcsetpgrp-test.c
      // buf afeter put this part, the status return wrong for the commands
      // this part must be understood better
      sigset_t set, oset;
      sigemptyset (&set);
      sigaddset (&set, SIGCHLD);
      sigaddset (&set, SIGTTOU);
      sigemptyset (&oset);
      sigprocmask (SIG_BLOCK, &set, &oset);
      tcsetpgrp(shell_terminal, pgid);
    }

    // Set the handling for job control signals back to the default
    signal (SIGINT, SIG_DFL);
    signal (SIGQUIT, SIG_DFL);
    signal (SIGTSTP, SIG_DFL);
    signal (SIGTTIN, SIG_DFL);
    signal (SIGTTOU, SIG_DFL);
    signal (SIGCHLD, SIG_DFL);
  }

  // set the standard input/output channels of the new process
  if (infile != STDIN_FILENO) {
    dup2 (infile, STDIN_FILENO);
    close (infile);
  }

  if (outfile != STDOUT_FILENO) {
    dup2 (outfile, STDOUT_FILENO);
    close (outfile);
  }

  if (errfile != STDERR_FILENO) {
    dup2 (errfile, STDERR_FILENO);
    close (errfile);
  }

  std::vector<std::string> args;

  if (sym_tab_.ExistsCmdAlias(args_[0])) {
    args = sym_tab_.GetCmdAlias(args_[0]);

    // append arguments
    for (size_t i = 1; i < args_.size(); i++) {
      args.push_back(args_[i]);
    }
  } else {
    // append arguments
    for (auto& e: args_) {
      args.push_back(e);
    }
  }

  CmdEntryPtr cmd = sym_tab_.LookupCmd(args[0]);

  CmdSharedError *err = (CmdSharedError*)
      shmat(EnvShell::instance()->shmid(), 0, 0);
  err->error = false;

  if (cmd) {
    try {
      LaunchCmd(cmd);
      exit(0);
    } catch (RunTimeError& e) {
      // set error on memory region
      err->error = true;
      err->except_code = static_cast<int>(e.code_);
      err->err_code = 0;
      int len = e.msg().length();
      len = len >= SETI_CMD_SIZE_MAX? SETI_CMD_SIZE_MAX-1: len;
      memcpy(err->err_str, e.msg().c_str(), len);
      exit(-1);
    }
  }

  char **argv = FillArgv(args);

  // Exec the new process
  execvp(argv[0], argv);

  // set error on memory region
  err->error = true;
  err->except_code = static_cast<int>(RunTimeError::ErrorCode::INVALID_COMMAND);
  err->err_code = errno;
  int len = std::strlen(argv[0]);
  len = len >= SETI_CMD_SIZE_MAX? SETI_CMD_SIZE_MAX-1: len;
  memcpy(err->err_str, argv[0], len);

  delete[] argv;

  // set \0 to end string
  err->err_str[len] = '\0';

  shmdt(err);
  exit(-1);
}

void Process::LaunchCmd(CmdEntryPtr cmd) {
  cmd->Exec(parent_, std::move(args_));
}

char** Process::FillArgv(const std::vector<std::string>& args) {
  char **argv;
  argv = new char*[args.size() + 1];

  for (int i = 0; i < args.size(); i++) {
    argv[i] = const_cast<char*>(args[i].data());
  }

  argv[args.size()] = NULL;

  return argv;
}

int Job::MarkProcessStatus(pid_t pid, int status) {
  if (pid > 0) {
    for (auto& p: process_) {
      if (p.pid_ == pid) {
        p.status_ = status;
        if (WIFSTOPPED (status)) {
          p.stopped_ = true;
        } else {
          p.completed_ = true;
        }
        return 0;
      }
    }
  } else if (pid == 0 || errno == ECHILD) {
    /* No processes ready to report.  */
    return -1;
  }

  return -1;
}

bool Job::JobIsStopped() {
  for (auto& p: process_) {
    if (!p.completed_ && !p.stopped_) {
      return false;
    }
  }

  return true;
}

bool Job::JobIsCompleted() {
  for (auto& p: process_) {
    if (!p.completed_) {
      return false;
    }
  }

  return true;
}

void Job::WaitForJob() {
  pid_t pid;
  int status;

  do {
    pid = waitpid (WAIT_ANY, &status, WUNTRACED);
  } while (!MarkProcessStatus(pid, status)
           && !JobIsStopped() && !JobIsCompleted());

  signal(SIGCHLD, SIG_IGN);
}

int Job::Status() {
  int status = 0;

  for (auto& p: process_) {
    status |= p.status_;
  }

  return status;
}

void Job::PutJobInForeground(int cont) {
  int shell_terminal = EnvShell::instance()->shell_terminal();
  struct termios* tmodes = EnvShell::instance()->shell_tmodes();
  pid_t shell_pgid = EnvShell::instance()->shell_pgid();

  // put the job into the foreground
  tcsetpgrp (shell_terminal, pgid_);

  // send the job a continue signal, if necessary
  if (cont) {
    tcsetattr (shell_terminal, TCSADRAIN, &shell_tmodes_);
    if (kill (- pgid_, SIGCONT) < 0) {
      perror ("kill (SIGCONT)");
    }
  }

  // wait for it to report
  WaitForJob();

  // put the shell back in the foreground
  tcsetpgrp(shell_terminal, shell_pgid);

  /* Restore the shell’s terminal modes.  */
  tcgetattr(shell_terminal, &shell_tmodes_);
  tcsetattr(shell_terminal, TCSADRAIN, tmodes);
}

void Job::PutJobInBackground(int cont) {
  /* Send the job a continue signal, if necessary.  */
  if (cont) {
    if (kill (-pgid_, SIGCONT) < 0)
      perror ("kill (SIGCONT)");
  }
}

void Job::LaunchInternalCmd(CmdEntryPtr cmd) {
  static_cast<CmdInEntry&>(*cmd).SetStdFd(stdout_, stderr_, stdin_);
  cmd->Exec(parent_, std::move(process_[0].args_));

  if (stdin_ != STDIN_FILENO) {
    close (stdin_);
  }

  if (stdout_ != STDOUT_FILENO) {
    close (stdout_);
  }

  if (stderr_ != STDERR_FILENO) {
    close (stderr_);
  }
}

void Job::LaunchJob(int foreground) {
  // executes commands that change self process status
  // commands like cd and exit must be executed this way
  if (process_.size() == 1) {
    CmdEntryPtr cmd = sym_tab_.LookupCmd(process_[0].args_[0]);

    if (cmd) {
      if (cmd->type() == CmdEntry::Type::kIn) {
        LaunchInternalCmd(cmd);
        return;
      }
    }
  }

  pid_t pid;
  int mypipe[2], infile, outfile;

  infile = stdin_;
  for (size_t i = 0; i < process_.size(); i++) {
    // set up pipes, if necessary
    if (i != (process_.size() - 1)) {
      if (pipe (mypipe) < 0) {
        perror ("pipe");
        exit (1);
      }

      outfile = mypipe[1];
    }
    else {
      outfile = stdout_;
    }

    // fork the child processes
    pid = fork ();
    if (pid == 0) {
      // this is the child process
      process_[i].LaunchProcess(infile, outfile, stderr_, pgid_, foreground);
    } else if (pid < 0) {
      // the fork failed
      perror ("fork");
      exit (1);
    } else {
      // this is the parent process
      process_[i].pid_ = pid;
      bool shell_is_interactive = EnvShell::instance()->shell_is_interactive();

      if (shell_is_interactive) {
        if (!pgid_) {
          pgid_ = pid;
        }

        setpgid(pid, pgid_);
      }
    }

    // clean up after pipes
    if (infile != stdin_)
      close (infile);
    if (outfile != stdout_)
      close (outfile);
    infile = mypipe[0];
  }

  bool shell_is_interactive = EnvShell::instance()->shell_is_interactive();

  if (!shell_is_interactive) {
    WaitForJob();
    CheckCmdError();
  } else if (foreground) {
    signal (SIGCHLD, SIG_DFL);
    PutJobInForeground(0);
    CheckCmdError();
  } else {
    PutJobInBackground(0);
  }
}

void Job::CheckCmdError() {
  // verify if there was some error executing command
  CmdSharedError *cmd_err = (CmdSharedError *) shmat(
        EnvShell::instance()->shmid(), 0, 0);

  CmdSharedError cpy_err;
  std::memcpy(&cpy_err, cmd_err, sizeof(CmdSharedError));

  shmdt(cmd_err);
  if (cpy_err.error) {
        throw RunTimeError(RunTimeError::ErrorCode::INVALID_COMMAND,
                           boost::format("%1%: %2%")%
                           cpy_err.err_str %strerror(cpy_err.err_code));
  }
}

}
}
