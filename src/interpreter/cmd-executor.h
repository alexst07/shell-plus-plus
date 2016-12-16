#ifndef SETI_CMD_EXEC_H
#define SETI_CMD_EXEC_H

#include <tuple>

#include "executor.h"

namespace setti {
namespace internal {

class CmdExecutor: public Executor {
 public:
  CmdExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  int Exec(CmdFull *node);
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

  std::tuple<std::string, int> Exec(CmdIoRedirect *node);
};


}
}

#endif  // SETI_CMD_EXEC_H

