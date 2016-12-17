#ifndef SETI_CMD_EXEC_H
#define SETI_CMD_EXEC_H

#include <string>
#include <memory>
#include <stack>
#include <list>
#include <functional>
#include <boost/variant.hpp>

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
    pipe_list_.push(std::move(v));
  }

  void Push(std::string&& cmd) {
    boost::variant<CmdIoRedirectData, std::string> v(std::move(cmd));
    pipe_list_.push(std::move(v));
  }

 private:
  std::stack<boost::variant<CmdIoRedirectData, std::string>> pipe_list_;
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


int ExecCmd(std::vector<std::string> &&args);

int WaitCmd(int pid);

}
}

#endif  // SETI_CMD_EXEC_H

