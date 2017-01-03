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

class CmdIoRedirectListExecutor: public Executor {
 public:
  CmdIoRedirectListExecutor(Executor* parent,
                            SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  int Exec(CmdIoRedirectList *node, bool background);

  std::tuple<int, std::string> Exec(CmdIoRedirectList *node);

  int GetInteger(Literal* integer);

  static std::string FileName(FilePathCmd* file_path);

  Job PrepareData(CmdIoRedirectList *node);
};

class CmdPipeSequenceExecutor: public Executor {
 public:
  CmdPipeSequenceExecutor(Executor* parent,
                            SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  int Exec(CmdPipeSequence *node, bool background);
  void InputFile(CmdIoRedirectList* file, Job &job);
  void OutputFile(CmdIoRedirectList* cmd_io, Job &job);
  void SelectInterface(CmdIoRedirect* io, Job& job, int fd);
  int GetInteger(Literal* integer);
  void AddCommand(Job& job, Cmd *cmd);
};

}
}

#endif  // SETI_CMD_EXECUTOR_H

