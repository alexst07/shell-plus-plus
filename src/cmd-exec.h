#ifndef SETI_CMD_EXEC_H
#define SETI_CMD_EXEC_H

#include <string>
#include <memory>
#include <stack>
#include <list>
#include <functional>
#include <boost/variant.hpp>
#include <termios.h>

#include "interpreter/abstract-obj.h"

namespace setti {
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

struct Process {
  Process(std::vector<std::string>&& args):args_(std::move(args)) {
    argv_ = new char*[args_.size() + 1];

    for (size_t i = 0; i < args_.size(); i++) {
      argv_[i] = const_cast<char*>(args_[i].data());
    }

    argv_[args_.size()] = NULL;
  }

  ~Process() {
    delete[] argv_;
  }

  Process(const Process& p) {
    args_ = p.args_;
    argv_ = new char*[args_.size() + 1];

    for (size_t i = 0; i < args_.size(); i++) {
      argv_[i] = const_cast<char*>(args_[i].data());
    }

    argv_[args_.size()] = NULL;

    pid_ = p.pid_;
    completed_ = p.completed_;
    stopped_ = p.stopped_;
    status_ = p.status_;
  }

  Process& operator=(const Process& p) {
    args_ = p.args_;

    delete[] argv_;
    argv_ = new char*[args_.size() + 1];

    for (size_t i = 0; i < args_.size(); i++) {
      argv_[i] = const_cast<char*>(args_[i].data());
    }

    argv_[args_.size()] = NULL;

    pid_ = p.pid_;
    completed_ = p.completed_;
    stopped_ = p.stopped_;
    status_ = p.status_;
  }

  Process(Process&& p) {
    args_ = std::move(p.args_);
    argv_ = p.argv_;
    p.argv_ = nullptr;
    pid_ = p.pid_;
    completed_ = p.completed_;
    stopped_ = p.stopped_;
    status_ = p.status_;
  }

  Process& operator=(Process&& p) {
    args_ = std::move(p.args_);
    argv_ = p.argv_;
    p.argv_ = nullptr;
    pid_ = p.pid_;
    completed_ = p.completed_;
    stopped_ = p.stopped_;
    status_ = p.status_;

    return *this;
  }

  void LaunchProcess (int infile, int outfile, int errfile);

  std::vector<std::string> args_;
  char **argv_;
  pid_t pid_;
  char completed_;
  char stopped_;
  int status_;
};


struct Job {
  Job(bool var_out_mode = false)
      : status_(0)
      , var_out_mode_(var_out_mode) {}

  void LaunchJob (int foreground);
  int MarkProcessStatus(pid_t pid, int status);
  int JobIsStopped();
  int JobIsCompleted();
  void WaitForJob();
  bool Status();
  void PutJobInForeground(int cont);
  void PutJobInBackground(int cont);

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
};

void LaunchProcess (char **argv, int infile, int outfile, int errfile);

int ExecCmd(std::vector<std::string> &&args);

int WaitCmd(int pid);

class BuildJob {
 public:
  BuildJob(CmdTable& cmd): cmd_(cmd) {}
  Job Build();

 private:
  CmdTable& cmd_;
};

}
}

#endif  // SETI_CMD_EXEC_H

