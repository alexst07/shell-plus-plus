#ifndef SETI_CMD_EXEC_H
#define SETI_CMD_EXEC_H

#include <string>
#include <memory>
#include <stack>
#include <list>

namespace setti {
namespace internal {

struct CmdIoData {
  std::string file_;
  int out_;
};

typedef CmdIoListData std::list<CmdIoData>;

struct CmdIoRedirectData {
  CmdIoListData io_list_;
  std::string cmd_;
};

typedef CmdPipeListData std::stack<CmdIoListData>;

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

class CmdExec {

};

}
}

#endif  // SETI_CMD_EXEC_H

