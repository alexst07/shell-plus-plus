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

  void ExecSimpleCmd(SimpleCmd *node, bool foreground);

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

  int Exec(CmdIoRedirectList *node, bool background);

  std::tuple<int, std::string> Exec(CmdIoRedirectList *node);

  int GetInteger(Literal* integer);

  std::string FileName(FilePathCmd* file_path);

  Job PrepareData(CmdIoRedirectList *node);

  int SelectFile(CmdIoData::Direction direction, const std::string& file_name);

  void CopyStdIo(int fd, CmdIoData::Direction direction, int iface, bool all);

  int ExecCmdIo(CmdIoRedirectData&& io_data, bool background);

  std::tuple<int, std::string> ExecCmdIoWithResult(CmdIoRedirectData&& io_data);
};

class CmdPipeSequenceExecutor: public Executor {
 public:
  CmdPipeSequenceExecutor(Executor* parent,
                            SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

//  CmdPipeListData Exec(CmdPipeSequence *node);
};

}
}

#endif  // SETI_CMD_EXECUTOR_H

