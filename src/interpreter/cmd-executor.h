#ifndef SETI_CMD_EXECUTOR_H
#define SETI_CMD_EXECUTOR_H

#include <tuple>

#include "executor.h"
#include "cmd-exec.h"

namespace setti {
namespace internal {

class CmdExecutor: public Executor {
 public:
  CmdExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  std::tuple<int, std::string> ExecGetResult(CmdFull *node);

  void Exec(CmdFull *node);

  void ExecSimpleCmd(SimpleCmd *node, bool background);

  std::tuple<int, std::string> ExecSimpleCmdWithResult(SimpleCmd *node);

  std::string CmdOutput() const;

 private:
  std::string cmd_output_;
};

class SimpleCmdExecutor: public Executor {
 public:
  SimpleCmdExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  std::vector<std::string> Exec(SimpleCmd *node);
};

class CmdIoRedirectExecutor: public Executor {
 public:
  CmdIoRedirectExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  CmdIoData Exec(CmdIoRedirect *node);

  CmdIoData::Direction SelectDirection(TokenKind kind);
};

class CmdIoRedirectListExecutor: public Executor {
 public:
  CmdIoRedirectListExecutor(Executor* parent,
                            SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  CmdIoRedirectData Exec(CmdIoRedirectList *node);
};

class CmdPipeSequenceExecutor: public Executor {
 public:
  CmdPipeSequenceExecutor(Executor* parent,
                            SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  CmdPipeListData Exec(CmdPipeSequence *node);
};

}
}

#endif  // SETI_CMD_EXECUTOR_H

