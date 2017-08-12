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
#include "interpreter/scope-executor.h"
#include "utils/scope-exit.h"

namespace shpp {
namespace internal {

Arguments::Arguments(std::vector<std::string>&& args)
    : aloc_glob_(false) {
  if (args.size() > 1) {
    globbuf_.gl_offs = 1;
    int flag = GLOB_NOMAGIC | GLOB_BRACE | GLOB_TILDE | GLOB_DOOFFS;

    for (int i = 1; i < args.size(); i++) {
      if (i > 1) {
        flag |= GLOB_APPEND;
      }

      glob(args[i].c_str(), flag, nullptr, &globbuf_);
      aloc_glob_ = true;
    }

    globbuf_.gl_pathv[0] = const_cast<char*>(args[0].c_str());

    // globbuf_.gl_pathc don't count the offset, so we need plus one
    // on the memory allocation, on the loop and in the nullptr assignment
    argv_ = new char*[globbuf_.gl_pathc + 2];

    for (int i = 0; i <= globbuf_.gl_pathc; i++) {
      argv_[i] = const_cast<char*>(globbuf_.gl_pathv[i]);
    }

    argv_[globbuf_.gl_pathc + 1] = nullptr;
    return;
  }

  argv_ = new char*[2];
  argv_[0] = const_cast<char*>(args[0].c_str());
  argv_[1] = nullptr;
}

Arguments::~Arguments() {
  if (argv_ != nullptr) {
    delete argv_;
  }

  if (aloc_glob_) {
    globfree (&globbuf_);
  }
}

char **Arguments::argsv() {
  return argv_;
}

std::vector<std::string> Arguments::args() {
  std::vector<std::string> arg_vec;

  int p = 0;
  while (argv_[p]) {
    arg_vec.push_back(std::string(argv_[p++]));
  }

  return arg_vec;
}

ProcessBase::ProcessBase(SymbolTableStack& sym_tab,
    std::vector<std::string>&& args, Executor* parent)
    : args_(std::move(args))
    , sym_tab_(sym_tab.MainTable())
    , completed_(false)
    , stopped_(false)
    , parent_(parent) {}

const std::vector<std::string>& ProcessBase::Args() const {
  return args_;
}

std::vector<std::string>& ProcessBase::Args() {
  return args_;
}

pid_t ProcessBase::pid() const {
  return pid_;
}

ProcessBase& ProcessBase::pid(pid_t v) {
  pid_ = v;
  return *this;
}

bool ProcessBase::completed() const {
  return completed_;
}

ProcessBase& ProcessBase::completed(bool v) {
  completed_ = v;
  return *this;
}

bool ProcessBase::stopped() const {
  return stopped_;
}

ProcessBase& ProcessBase::stopped(bool v) {
  stopped_ = v;
  return *this;
}

int ProcessBase::status() const {
  return status_;
}

ProcessBase& ProcessBase::status(int v) {
  status_ = v;
  return *this;
}

Process::Process(SymbolTableStack& sym_tab, std::vector<std::string>&& args,
    Executor* parent):ProcessBase(sym_tab, std::move(args), parent) {}

inline void Tcsetpgrp(pid_t pid) {
  // executes only when called from main process
  if (getpid() == EnvShell::instance()->shell_pid()) {
    int shell_terminal = EnvShell::instance()->shell_terminal();
    tcsetpgrp(shell_terminal, pid);
  }
}

inline void Tcsetattr(struct termios* tmodes) {
    // executes only when called from main process
  if (getpid() == EnvShell::instance()->shell_pid()) {
    int shell_terminal = EnvShell::instance()->shell_terminal();
    tcsetattr(shell_terminal, TCSADRAIN, tmodes);
  }
}

inline void CloseFileDescriptor(int fd) {
  if (fd != STDOUT_FILENO && fd != STDERR_FILENO) {
    close(fd);
  }
}

void SetUpFilesDescriptor(int infile, int outfile, int errfile, pid_t pgid,
                            bool foreground) {
  pid_t pid;
  int shell_terminal = EnvShell::instance()->shell_terminal();
  int shell_is_interactive = isatty(shell_terminal);

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
      Tcsetpgrp(pgid);
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
    dup2(infile, STDIN_FILENO);
    CloseFileDescriptor(infile);
  }

  if (outfile == errfile) {
    // when redirect stdout and stderr to the same file
    if (errfile != STDERR_FILENO) {
      dup2(errfile, STDERR_FILENO);
    }

    if (outfile != STDOUT_FILENO) {
      dup2(outfile, STDOUT_FILENO);
    }

    CloseFileDescriptor(outfile);
  } else {
    // when redirect stdout and stderr to different file
    if (outfile != STDOUT_FILENO) {
      dup2(outfile, STDOUT_FILENO);
      CloseFileDescriptor(outfile);
    }

    if (errfile != STDERR_FILENO) {
      dup2(errfile, STDERR_FILENO);
      CloseFileDescriptor(errfile);
    }
  }
}

void Process::LaunchProcess(int infile, int outfile, int errfile, pid_t pgid,
                            bool foreground) {
  SetUpFilesDescriptor(infile, outfile, errfile, pgid, foreground);

  std::vector<std::string> args;

  if (sym_tab().ExistsCmdAlias(Args()[0])) {
    args = sym_tab().GetCmdAlias(Args()[0]);

    // append arguments
    for (size_t i = 1; i < Args().size(); i++) {
      args.push_back(Args()[i]);
    }
  } else {
    // append arguments
    for (auto& e: Args()) {
      args.push_back(e);
    }
  }

  CmdEntryPtr cmd = sym_tab().LookupCmd(args[0]);

  CmdSharedError *err = (CmdSharedError*)
      shmat(EnvShell::instance()->shmid(), 0, 0);
  err->error = false;

  Arguments glob_args(std::move(args));

  if (cmd) {
    try {
      LaunchCmd(cmd, std::move(glob_args.args()));
      exit(0);
    } catch (RunTimeError& e) {
      // set error on memory region
      err->error = true;
      err->except_code = static_cast<int>(e.code_);
      err->err_code = 0;
      int len = e.msg().length();
      len = len >= SHPP_CMD_SIZE_MAX? SHPP_CMD_SIZE_MAX-1: len;
      memcpy(err->err_str, e.msg().c_str(), len);
      exit(-1);
    }
  }

  int p = 0;

  // Exec the new process
  execvp(glob_args.argsv()[0], glob_args.argsv());

  // set error on memory region
  err->error = true;
  err->except_code = static_cast<int>(RunTimeError::ErrorCode::INVALID_COMMAND);
  err->err_code = errno;
  int len = std::strlen(glob_args.argsv()[0]);
  len = len >= SHPP_CMD_SIZE_MAX? SHPP_CMD_SIZE_MAX-1: len;
  memcpy(err->err_str, glob_args.argsv()[0], len);

  // set \0 to end string
  err->err_str[len] = '\0';

  shmdt(err);
  exit(-1);
}

void Process::LaunchCmd(CmdEntryPtr cmd, std::vector<std::string>&& args) {
  cmd->Exec(parent(), std::move(args));
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

ProcessSubShell::ProcessSubShell(SymbolTableStack& sym_tab, SubShell* sub_shell,
    Executor* parent)
    : ProcessBase(sym_tab, std::move(std::vector<std::string>()), parent)
    , sub_shell_(sub_shell) {}

void ProcessSubShell::LaunchProcess(int infile, int outfile, int errfile,
    pid_t pgid, bool foreground) {
  SetUpFilesDescriptor(infile, outfile, errfile, pgid, foreground);

  // create a simple symbol table
  SymbolTablePtr table = SymbolTable::Create();

  // push a simple symbol table to stack
  sym_tab().Push(table);

  BlockExecutor executor(parent(), sym_tab());

  // scope exit case an excpetion thrown
  auto cleanup = MakeScopeExit([&]() {
    executor.ExecuteDeferStack();
    sym_tab().Pop();
  });
  IgnoreUnused(cleanup);

  // executes the block of sub-shell
  executor.Exec(sub_shell_->block());

  exit(0);
}

int Job::MarkProcessStatus(pid_t pid, int status) {
  if (pid > 0) {
    for (auto& p: process_) {
      if (p->pid() == pid) {
        p->status(status);

        if (WIFSTOPPED (status)) {
          p->stopped(true);
        } else {
          p->completed(true);
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

Job& Job::Stdin(int stdin) {
  stdin_ = stdin;
  return *this;
}

Job& Job::Stdout(int stdout) {
  stdout_ = stdout;
  return *this;
}

Job& Job::Stderr(int stderr) {
  stderr_ = stderr;
  return *this;
}

Job& Job::AddProcess(std::unique_ptr<ProcessBase>&& p) {
  process_.push_back(std::move(p));
  return *this;
}

bool Job::JobIsStopped() {
  for (auto& p: process_) {
    if (!p->completed() && !p->stopped()) {
      return false;
    }
  }

  return true;
}

bool Job::JobIsCompleted() {
  for (auto& p: process_) {
    if (!p->completed()) {
      return false;
    }
  }

  return true;
}

void Job::WaitForJob() {
  pid_t pid;
  int status = 0;

  // return signal SIGCHLD for default action to get the result of
  // child process
  signal(SIGCHLD, SIG_DFL);

  do {
    pid = waitpid (WAIT_ANY, &status, WUNTRACED);

    if (EnvShell::instance()->interective_exec()) {
      // if it is interactive execution, when receive a SIGINT, stop to
      // execute the blok, the action is like break statement
      if (WIFSIGNALED(status) && (WTERMSIG(status) == SIGINT ||
          WTERMSIG(status) == SIGQUIT)) {
        parent_->set_stop(Executor::StopFlag::kBreak);
        return;
      }
    } else {
      // if execution isn't interective, and the last command was terminated
      // with SIGINT, so, stop the execution of program
      if (WIFSIGNALED(status) && (WTERMSIG(status) == SIGINT ||
          WTERMSIG(status) == SIGQUIT)) {
        // Deliver the SIGINT or SIGQUIT signal to ourself since we're not interactive.
        struct sigaction act;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        act.sa_handler = SIG_DFL;
        sigaction(SIGINT, &act, 0);
        sigaction(SIGQUIT, &act, 0);
        kill(getpid(), WTERMSIG(status));
      }
    }
  } while (!MarkProcessStatus(pid, status)
           && !JobIsStopped() && !JobIsCompleted());

  // ignore signal SIGCHLD to avoid any zombie process
  signal(SIGCHLD, SIG_IGN);
}

int Job::Status() {
  int status = 0;

  for (auto& p: process_) {
    status |= p->status();
  }

  return status;
}

void Job::PutJobInForeground(int cont) {
  int shell_terminal = EnvShell::instance()->shell_terminal();
  struct termios* tmodes = EnvShell::instance()->shell_tmodes();
  pid_t shell_pgid = EnvShell::instance()->shell_pgid();

  Tcsetpgrp(pgid_);

  // send the job a continue signal, if necessary
  if (cont) {
    Tcsetattr(&shell_tmodes_);

    if (kill (- pgid_, SIGCONT) < 0) {
      perror ("kill (SIGCONT)");
    }
  }

  // wait for it to report
  WaitForJob();

  // if we are on subshell, so the shell can't back to foreground
  if (getpid() != EnvShell::instance()->shell_pid()) {
    return;
  }

  // put the shell back in the foreground
  Tcsetpgrp(shell_pgid);

  /* Restore the shell’s terminal modes.  */
  tcgetattr(shell_terminal, &shell_tmodes_);
  Tcsetattr(tmodes);
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

  Arguments glob_args(std::move(process_[0]->Args()));

  cmd->Exec(parent_, std::move(glob_args.args()));

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
  int shell_terminal = EnvShell::instance()->shell_terminal();

  // executes commands that change self process status
  // commands like cd and exit must be executed this way
  if (process_.size() == 1) {
    // sub-shell has size of args equal to 0, so if it is not
    // a sub-shell so try to check if it is a buitin command
    if (process_[0]->Args().size() > 0) {
      CmdEntryPtr cmd = sym_tab_.LookupCmd(process_[0]->Args()[0]);

      if (cmd) {
        if (cmd->type() == CmdEntry::Type::kIn) {
          // execute the builtin command without fork a new process
          LaunchInternalCmd(cmd);
          return;
        }
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
      process_[i]->LaunchProcess(infile, outfile, stderr_, pgid_, foreground);
    } else if (pid < 0) {
      // the fork failed
      perror ("fork");
      exit (1);
    } else {
      // this is the parent process
      process_[i]->pid(pid);
      int shell_is_interactive = isatty(shell_terminal);

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

  int shell_is_interactive = isatty(shell_terminal);

  if (!shell_is_interactive) {
    WaitForJob();
    CheckCmdError();
  } else if (foreground) {
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
