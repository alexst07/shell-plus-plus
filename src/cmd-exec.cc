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

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "interpreter/cmd-executor.h"

namespace seti {
namespace internal {

int ExecCmd(std::vector<std::string>&& args) {
  char** p_args = new char*[args.size() + 1];

  for (size_t i = 0; i < args.size(); i++) {
    p_args[i] = const_cast<char*>(args[i].data());
    std::cout << "::" << p_args[i];
  }

  p_args[args.size()] = NULL;

  int ret = execvp(p_args[0], p_args);

  delete[] p_args;

  return ret;
}

int WaitCmd(int pid) {
  int status;

  waitpid(pid,&status,0);
  return status;
}

void Process::LaunchProcess(int infile, int outfile, int errfile) {
  /* Set the standard input/output channels of the new process.  */
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

  CmdEntryPtr cmd = sym_tab_.LookupCmd(args_[0]);

  if (cmd) {
    LaunchCmd(cmd);
    exit(0);
  }

  // Exec the new process
  execvp (argv_[0], argv_);

  // if some error ocurred on exec, throw the exception
  throw RunTimeError(RunTimeError::ErrorCode::INVALID_COMMAND,
                     boost::format("%1%: command not found")%argv_[0]);

}

void Process::LaunchCmd(CmdEntryPtr cmd) {
  if (cmd->type() == CmdEntry::Type::kDecl) {
    CmdDeclEntry& cmd_ref = static_cast<CmdDeclEntry&>(*cmd);
    cmd_ref.Exec(parent_, std::move(args_));
  }
}

int Job::MarkProcessStatus(pid_t pid, int status) {
  if (pid > 0) {
    for (auto& p: process_) {
      if (p.pid_ == pid) {
        p.status_ = status;
        if (WIFSTOPPED (status)) {
          p.stopped_ = 1;
        } else {
          p.completed_ = 1;
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

int Job::JobIsStopped() {
  for (auto& p: process_) {
    if (!p.completed_ && !p.stopped_)
      return 0;
  }

  return 1;
}

int Job::JobIsCompleted() {
  for (auto& p: process_) {
    if (!p.completed_)
      return 0;
  }

  return 1;
}

void Job::WaitForJob() {
  if (!wait_) {
    return;
  }

  int status;
  pid_t pid;

  do {
    pid = waitpid (WAIT_ANY, &status, WUNTRACED);
    status_ |= status;
  } while (!MarkProcessStatus(pid, status)
           && !JobIsStopped() && !JobIsCompleted());
}

int Job::Status() {
  int status = 0;

  for (auto& p: process_) {
    status |= p.status_;
  }

  return status;
}

void Job::PutJobInForeground(int cont) {
  /* Put the job into the foreground.  */
  tcsetpgrp (shell_terminal_, pgid_);

  /* Send the job a continue signal, if necessary.  */
  if (cont) {
    tcsetattr (shell_terminal_, TCSADRAIN, &tmodes_);
    if (kill (- pgid_, SIGCONT) < 0) {
      perror ("kill (SIGCONT)");
    }
  }

  /* Wait for it to report.  */
  WaitForJob();

  /* Put the shell back in the foreground.  */
  tcsetpgrp(shell_terminal_, shell_pgid_);

  /* Restore the shell’s terminal modes.  */
  tcgetattr(shell_terminal_, &tmodes_);
  tcsetattr(shell_terminal_, TCSADRAIN, &shell_tmodes_);
}

void Job::PutJobInBackground(int cont) {
  /* Send the job a continue signal, if necessary.  */
  if (cont) {
    if (kill (-pgid_, SIGCONT) < 0)
      perror ("kill (SIGCONT)");
  }
}

void Job::LaunchJob(int foreground) {
  pid_t pid;
  int mypipe[2], infile, outfile;

  infile = stdin_;
  for (size_t i = 0; i < process_.size(); i++) {
    /* Set up pipes, if necessary.  */
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

    /* Fork the child processes.  */
    pid = fork ();
    if (pid == 0) {
      /* This is the child process.  */
      process_[i].LaunchProcess(infile, outfile, stderr_);
    } else if (pid < 0) {
      /* The fork failed.  */
      perror ("fork");
      exit (1);
    } else {
      /* This is the parent process.  */
      process_[i].pid_ = pid;
    }

    /* Clean up after pipes.  */
    if (infile != stdin_)
      close (infile);
    if (outfile != stdout_)
      close (outfile);
    infile = mypipe[0];
  }

  if (!shell_is_interactive_)
    WaitForJob();
  else if (foreground)
    PutJobInForeground(0);
  else
    PutJobInBackground(0);
}


}
}
