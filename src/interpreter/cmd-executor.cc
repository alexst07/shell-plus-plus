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

#include "cmd-executor.h"

#include <unistd.h>
#include <unistd.h>
#include <fcntl.h>
#include <climits>
#include <cstdio>
#include <boost/algorithm/string.hpp>

#include "expr-executor.h"
#include "objects/str-object.h"
#include "stmt-executor.h"
#include "scope-executor.h"
#include "utils/scope-exit.h"
#include "objects/obj-type.h"

namespace seti {
namespace internal {

void SetFdAsync(int fd) {
  int flags;
  if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
          flags = 0;
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

std::tuple<std::string, std::string> ReadPipe(int pipe_out, int pipe_err) {
  char buf[PIPE_BUF];
  char buf_err[PIPE_BUF];

  int rd = 0;
  int rd_err = 0;

  SetFdAsync(pipe_out);
  SetFdAsync(pipe_err);

  rd = read(pipe_out, buf, PIPE_BUF);
  rd_err = read(pipe_err, buf_err, PIPE_BUF);

  std::string str_out = "";
  std::string str_err = "";

  if (rd > 0) {
    buf[rd] = '\0';
    str_out += buf;
  }

  if (rd_err > 0) {
    buf_err[rd_err] = '\0';
    str_err += buf_err;
  }

  return std::tuple<std::string, std::string>(str_out, str_err);
}

void CmdDeclEntry::Exec(Executor* parent, std::vector<std::string>&& args) {
  // it is the table function
  SymbolTablePtr table =
      SymbolTable::Create(SymbolTable::TableType::FUNC_TABLE);

  // main symbol of function
  symbol_table_.Push(table, false);

  BlockExecutor executor(parent, symbol_table_, true);

  // scope exit case an excpetion thrown
  auto cleanup = MakeScopeExit([&]() {
    executor.ExecuteDeferStack();
    symbol_table_.Pop();
  });
  IgnoreUnused(cleanup);

  std::vector<ObjectPtr> vec_args;

  for (auto& arg: args) {
    ObjectFactory obj_factory(symbol_table_);
    ObjectPtr str_obj(obj_factory.NewString(arg));
    vec_args.push_back(str_obj);
  }

  ObjectFactory obj_factory(symbol_table_);
  ObjectPtr array_obj(obj_factory.NewArray(std::move(vec_args)));

  // arguments as passed to command as an array called args
  symbol_table_.SetEntry("args", array_obj);

  executor.Exec(start_node_.get());
}

CmdExprData CmdExecutor::ExecGetResult(CmdFull *node) {
  return ExecCmdGetResult(node->cmd());
}

CmdExprData CmdExecutor::ExecCmdGetResult(Cmd *node)
try {
  switch (node->type()) {
    case AstNode::NodeType::kSimpleCmd: {
      return ExecSimpleCmdWithResult(static_cast<SimpleCmd*>(node));
    } break;

    case AstNode::NodeType::kCmdIoRedirectList: {
      CmdIoRedirectListExecutor cmd_io(this, symbol_table_stack());
      return cmd_io.Exec(static_cast<CmdIoRedirectList*>(node));
    } break;

    case AstNode::NodeType::kCmdPipeSequence: {
      CmdPipeSequenceExecutor cmd_pipe(this, symbol_table_stack());
      return cmd_pipe.Exec(static_cast<CmdPipeSequence*>(node));
    } break;

    case AstNode::NodeType::kCmdAndOr: {
      CmdAndOr* cmd = static_cast<CmdAndOr*>(node);
      return ExecCmdBinOp(cmd);
    } break;

    default: {
      throw RunTimeError(RunTimeError::ErrorCode::INVALID_OPCODE,
                         boost::format("invalid command ast"));
    }
  }
} catch (RunTimeError& e) {
  throw RunTimeError(e.err_code(), e.msg(), node->pos());
}

CmdExprData CmdExecutor::ExecCmdBinOp(CmdAndOr* cmd) {
  CmdExprData lcmd = ExecCmdGetResult(cmd->cmd_left());

  if (cmd->kind() == TokenKind::AND) {
    if (std::get<0>(lcmd) == 0) {
      CmdExprData rcmd = ExecCmdGetResult(cmd->cmd_right());
      std::string str_out = std::get<1>(lcmd) + std::get<1>(rcmd);
      std::string str_err = std::get<2>(lcmd) + std::get<2>(rcmd);
      return CmdExprData(std::get<0>(rcmd),  str_out, str_err);
    }

    std::string str_out = std::get<1>(lcmd);
    std::string str_err = std::get<2>(lcmd);

    // -1 set error on status of operation
    return CmdExprData(-1,  str_out, str_err);
  } else /*TokenKind::OR*/ {
    if (std::get<0>(lcmd) == 0) {
      std::string str_out = std::get<1>(lcmd);
      std::string str_err = std::get<2>(lcmd);
      // -1 set error on status of operation
      return CmdExprData(std::get<0>(lcmd),  str_out, str_err);
    }

    CmdExprData rcmd = ExecCmdGetResult(cmd->cmd_right());
    std::string str_out = std::get<1>(lcmd) + std::get<1>(rcmd);
    std::string str_err = std::get<2>(lcmd) + std::get<2>(rcmd);
    return CmdExprData(std::get<0>(rcmd),  str_out, str_err);
  }
}

int CmdExecutor::ExecCmdBinOp(CmdAndOr* cmd, bool wait) {
  int lcmd = ExecCmd(cmd->cmd_left(), wait);

  if (cmd->kind() == TokenKind::AND) {
    if (lcmd == 0) {
      int rcmd = ExecCmd(cmd->cmd_right(), wait);
      return rcmd;
    }

    return lcmd;
  }

  /*TokenKind::OR*/
  if (lcmd == 0) {
    return lcmd;
  }

  int rcmd = ExecCmd(cmd->cmd_right(), wait);
  return rcmd;
}

int CmdExecutor::Exec(CmdFull *node) {
  bool wait = !node->background();
  return ExecCmd(node->cmd(), wait);
}

int CmdExecutor::ExecCmd(Cmd *node, bool wait)
try {
  switch (node->type()) {
    case AstNode::NodeType::kSimpleCmd: {
      return ExecSimpleCmd(static_cast<SimpleCmd*>(node), wait);
    } break;

    case AstNode::NodeType::kCmdIoRedirectList: {
      CmdIoRedirectListExecutor cmd_io(this, symbol_table_stack());
      return cmd_io.Exec(static_cast<CmdIoRedirectList*>(node), wait);
    } break;

    case AstNode::NodeType::kCmdPipeSequence: {
      CmdPipeSequenceExecutor cmd_pipe(this, symbol_table_stack());
      return cmd_pipe.Exec(static_cast<CmdPipeSequence*>(node), wait);
    } break;

    case AstNode::NodeType::kCmdAndOr: {
      CmdAndOr* cmd = static_cast<CmdAndOr*>(node);
      if (wait) {
        // wait all command
        return ExecCmdBinOp(cmd, true);
      } else {
        // as one command must wait for other to know the result
        // but the system cant waint, so, create a new process
        // to execute the whole command
        pid_t pid;
        pid = fork ();
        if (pid == 0) {
          int r = ExecCmdBinOp(cmd, true);
          exit(r);
        }

        return 0;
      }
    } break;

    default: {
      throw RunTimeError(RunTimeError::ErrorCode::INVALID_OPCODE,
                         boost::format("invalid command ast"));
    }
  }
} catch (RunTimeError& e) {
  throw RunTimeError(e.err_code(), e.msg(), node->pos());
}

int CmdExecutor::ExecSimpleCmd(SimpleCmd *node, bool wait) {
  SimpleCmdExecutor simple_cmd(this, symbol_table_stack());

  std::vector<std::string> cmd_args = simple_cmd.Exec(node);

  Job job(symbol_table_stack());
  Process p(symbol_table_stack(), std::move(cmd_args));
  job.process_.push_back(p);
  job.shell_is_interactive_ = 0;
  job.stderr_ = STDERR_FILENO;
  job.stdout_ = STDOUT_FILENO;
  job.stdin_ = STDIN_FILENO;
  job.wait_ = wait;
  job.LaunchJob(wait);

  if (wait) {
    return job.Status();
  } else {
    // if not wait, return as process success
    return 0;
  }
}

CmdExprData CmdExecutor::ExecSimpleCmdWithResult(
    SimpleCmd *node) {
  SimpleCmdExecutor simple_cmd(this, symbol_table_stack());

  std::vector<std::string> cmd_args = simple_cmd.Exec(node);

  const int READ = 0;
  const int WRITE = 1;

  int pipettes[2];
  int pipe_err[2];

  pipe(pipettes);
  pipe(pipe_err);

  Job job(symbol_table_stack());
  Process p(symbol_table_stack(), std::move(cmd_args));
  job.process_.push_back(p);
  job.shell_is_interactive_ = 0;
  job.stderr_ = pipe_err[WRITE];
  job.stdout_ = pipettes[WRITE];
  job.stdin_ = STDIN_FILENO;
  job.wait_ = true;
  job.LaunchJob(true);

  std::string str_out;
  std::string str_err;

  std::tie(str_out, str_err) = ReadPipe(pipettes[READ], pipe_err[READ]);

  close(pipettes[READ]);
  close(pipe_err[READ]);

  return CmdExprData(job.Status(), str_out, str_err);
}

////////////////////////////////////////////////////////////////////////////////

std::vector<std::string> SimpleCmdExecutor::Exec(SimpleCmd *node) {
  std::vector<AstNode*> pieces = node->children();
  std::vector<std::string> cmd;

  // variables used by cmd pieces
  std::string str_part = "";
  bool blank_after = false;
  bool is_cmd_piece = false;

  for (AstNode* piece: pieces) {
    if (piece->type() == AstNode::NodeType::kCmdPiece) {
      is_cmd_piece = true;
      CmdPiece* cmd_part = static_cast<CmdPiece*>(piece);

      str_part += cmd_part->cmd_str();
      blank_after = cmd_part->blank_after();

      if (blank_after) {
        cmd.push_back(str_part);
        str_part = "";
      }
    } else if (piece->type() == AstNode::NodeType::kCmdValueExpr) {
      is_cmd_piece = true;

      // handle expression inside the command, ex: ls ${expr + 2}
      CmdValueExpr* cmd_expr = static_cast<CmdValueExpr*>(piece);
      str_part +=  ResolveCmdExpr(this, cmd_expr);
      blank_after = cmd_expr->blank_after();

      if (blank_after) {
        cmd.push_back(str_part);
        str_part = "";
      }
    } else {
      throw RunTimeError(RunTimeError::ErrorCode::INVALID_OPCODE,
                         boost::format("invalid command ast"));
    }
  }

  // if the cmd doesn't finish with blank space, put its content on vector
  if (!blank_after && is_cmd_piece) {
    cmd.push_back(str_part);
  }

  return cmd;
}

////////////////////////////////////////////////////////////////////////////////
int CmdIoRedirectListExecutor::GetInteger(Literal* integer) {
  return boost::get<int>(integer->value());
}

std::string CmdIoRedirectListExecutor::FileName(Executor* parent,
                                                FilePathCmd* file_path) {
  std::vector<AstNode*> pieces = file_path->children();
  std::string str_part = "";

  for (AstNode* piece: pieces) {
    if (piece->type() == AstNode::NodeType::kCmdPiece) {
      CmdPiece* cmd_part = static_cast<CmdPiece*>(piece);

      str_part += cmd_part->cmd_str();

      if (cmd_part->blank_after()) {
        str_part += " ";
      }
    } else if(piece->type() == AstNode::NodeType::kCmdValueExpr) {
      CmdValueExpr* cmd_expr = static_cast<CmdValueExpr*>(piece);
      str_part += ResolveCmdExpr(parent, cmd_expr);

      if (cmd_expr->blank_after()) {
        str_part += " ";
      }
    } else {
      throw RunTimeError(RunTimeError::ErrorCode::INVALID_OPCODE,
                         boost::format("invalid command ast"));
    }
  }

  boost::trim(str_part);

  return str_part;
}

void CmdIoRedirectListExecutor::PrepareData(Job& job, CmdIoRedirectList *node) {
  // iterate over redirect io list
  std::vector<CmdIoRedirect*> cmd_io_list = node->children();
  for (auto& l : cmd_io_list) {
    int fd;

    std::string file_name = FileName(this, l->file_path_cmd());

    if (l->kind() == TokenKind::GREATER_THAN) {
      fd = CreateFile(file_name);
    } else if (l->kind() == TokenKind::SAR) {
      fd = AppendFile(file_name);
    } else if (l->kind() == TokenKind::LESS_THAN) {
      fd = ReadFile(file_name);
      job.stdin_ = fd;
    } else if (l->kind() == TokenKind::SHL) {
      fd = Var2Pipe(file_name, symbol_table_stack());
      job.stdin_ = fd;
    }

    if (l->all()) {
      job.stdout_ = fd;
      job.stderr_ = fd;
    } else {
      if (l->has_integer()) {
        int num = GetInteger(l->integer());
        // 2 is the error interface
        if (num == 2) {
          job.stderr_ = fd;
        } else if (num == 1) {
          job.stdout_ = fd;
        }
      } else {
        if (l->kind() == TokenKind::GREATER_THAN ||
            l->kind() == TokenKind::SAR) {
          job.stdout_ = fd;
        }
      }
    }
  }

  // io command can only have simple command
  if (node->cmd()->type() != AstNode::NodeType::kSimpleCmd) {
    throw RunTimeError(RunTimeError::ErrorCode::INVALID_OPCODE,
                       boost::format("invalid command ast"));
  }

  SimpleCmdExecutor simple_cmd(this, symbol_table_stack());
  std::vector<std::string> cmd_args = simple_cmd.Exec(static_cast<SimpleCmd*>(
      node->cmd()));

  Process p(symbol_table_stack(), std::move(cmd_args));
  job.process_.push_back(std::move(p));
}

int CmdIoRedirectListExecutor::Exec(CmdIoRedirectList *node, bool wait) {
  // starts job struct
  Job job(symbol_table_stack());
  job.shell_is_interactive_ = 0;
  job.stderr_ = STDERR_FILENO;
  job.stdout_ = STDOUT_FILENO;
  job.stdin_ = STDIN_FILENO;
  job.wait_ = wait;

  PrepareData(job, node);
  job.LaunchJob(!wait);

  return job.Status();
}

CmdExprData CmdIoRedirectListExecutor::Exec(
    CmdIoRedirectList *node) {
  const int READ = 0;
  const int WRITE = 1;

  int pipettes[2];
  int pipe_err[2];

  pipe(pipettes);
  pipe(pipe_err);

  Job job(symbol_table_stack());
  job.shell_is_interactive_ = 0;
  job.stderr_ = pipe_err[WRITE];
  job.stdout_ = pipettes[WRITE];
  job.stdin_ = STDIN_FILENO;
  job.wait_ = true;

  PrepareData(job, node);
  job.LaunchJob(true);

  std::string str_out, str_err;

  std::tie(str_out, str_err) = ReadPipe(pipettes[READ], pipe_err[READ]);

  close(pipettes[READ]);
  close(pipe_err[READ]);

  return CmdExprData(job.Status(), str_out, str_err);
}

////////////////////////////////////////////////////////////////////////////////
void CmdPipeSequenceExecutor::InputFile(CmdIoRedirectList* file, Job& job) {
  for (auto& io: file->children()) {
    std::string file_name =
        CmdIoRedirectListExecutor::FileName(this, io->file_path_cmd());

    int fd;
    // get only the input file
    if (io->kind() == TokenKind::LESS_THAN) {
      fd = ReadFile(file_name);
      job.stdin_ = fd;
    }
  }
}

int CmdPipeSequenceExecutor::GetInteger(Literal* integer) {
  return boost::get<int>(integer->value());
}

void CmdPipeSequenceExecutor::SelectInterface(CmdIoRedirect* io, Job& job,
                                              int fd) {
  if (io->all()) {
    job.stdout_ = fd;
    job.stderr_ = fd;
  } else {
    if (io->has_integer()) {
      int num = GetInteger(io->integer());
      // 2 is the error interface
      if (num == 2) {
        job.stderr_ = fd;
      } else if (num == 1) {
        job.stdout_ = fd;
      }
    } else {
      job.stdout_ = fd;
    }
  }
}

void CmdPipeSequenceExecutor::OutputFile(CmdIoRedirectList* cmd_io, Job &job) {
  for (auto& io: cmd_io->children()) {
    int fd;
    std::string file_name =
        CmdIoRedirectListExecutor::FileName(this, io->file_path_cmd());

    if (io->kind() == TokenKind::GREATER_THAN) {
      fd = CreateFile(file_name);
      SelectInterface(io, job, fd);
    } else if (io->kind() == TokenKind::SAR) {
      fd = AppendFile(file_name);
      SelectInterface(io, job, fd);
    }
  }
}

void CmdPipeSequenceExecutor::AddCommand(Job& job, Cmd* cmd) {
  // io command can only have simple command
  if (cmd->type() != AstNode::NodeType::kSimpleCmd) {
    throw RunTimeError(RunTimeError::ErrorCode::INVALID_OPCODE,
                       boost::format("invalid command ast"));
  }

  SimpleCmdExecutor simple_cmd(this, symbol_table_stack());
  std::vector<std::string> cmd_args =
      simple_cmd.Exec(static_cast<SimpleCmd*>(cmd));

  Process p(symbol_table_stack(), std::move(cmd_args));
  job.process_.push_back(std::move(p));
}

void CmdPipeSequenceExecutor::PopulateCmd(Job& job, CmdPipeSequence *node) {
  std::vector<Cmd*> cmds = node->cmds();
  int i = 0;

  for (auto& cmd: cmds) {
    if (i == 0) {
      // only in the first pipe can have input file
      if (cmd->type() == AstNode::NodeType::kCmdIoRedirectList) {
        CmdIoRedirectList* cmd_io = static_cast<CmdIoRedirectList*>(cmd);
        InputFile(cmd_io, job);
        AddCommand(job, cmd_io->cmd());
      } else {
        AddCommand(job, cmd);
      }
    } else if (i == (cmds.size() - 1)) {
      // the last pipe can have output file
      if (cmd->type() == AstNode::NodeType::kCmdIoRedirectList) {
        CmdIoRedirectList* cmd_io = static_cast<CmdIoRedirectList*>(cmd);
        OutputFile(cmd_io, job);
        AddCommand(job, cmd_io->cmd());
      } else {
        AddCommand(job, cmd);
      }
    } else {
      AddCommand(job, cmd);
    }

    i++;
  }
}

int CmdPipeSequenceExecutor::Exec(CmdPipeSequence *node, bool wait) {
  Job job(symbol_table_stack());
  job.shell_is_interactive_ = 0;
  job.stderr_ = STDERR_FILENO;
  job.stdout_ = STDOUT_FILENO;
  job.stdin_ = STDIN_FILENO;
  job.wait_ = wait;

  PopulateCmd(job, node);

  job.LaunchJob(wait);

  return job.Status();
}

CmdExprData CmdPipeSequenceExecutor::Exec(CmdPipeSequence *node) {
  const int READ = 0;
  const int WRITE = 1;

  int pipettes[2];
  int pipe_err[2];

  pipe(pipettes);
  pipe(pipe_err);

  Job job(symbol_table_stack());
  job.shell_is_interactive_ = 0;
  job.stderr_ = pipe_err[WRITE];
  job.stdout_ = pipettes[WRITE];
  job.stdin_ = STDIN_FILENO;
  job.wait_ = true;

  PopulateCmd(job, node);
  job.LaunchJob(true);

  std::string str_out = "";
  std::string str_err = "";

  std::tie(str_out, str_err) = ReadPipe(pipettes[READ], pipe_err[READ]);

  close(pipettes[READ]);
  close(pipe_err[READ]);

  return CmdExprData(job.Status(), str_out, str_err);
}

std::string ResolveCmdExpr(Executor* parent, CmdValueExpr* cmd_expr) {
  ExpressionExecutor expr(parent, parent->symbol_table_stack());
  ObjectPtr obj = expr.Exec(cmd_expr->expr());

  // get cmd method overload
  ObjectPtr str_obj(obj->ObjCmd());

  if (str_obj->type() != Object::ObjectType::STRING) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("object must be string"));
  }

  std::string part = static_cast<StringObject&>(*str_obj).value();
  return part;
}

int CreateFile(std::string file_name) {
  int fd = open(file_name.c_str(), O_CREAT | O_WRONLY | O_TRUNC,
                S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR | S_IROTH);

  if (fd > 0) {
    return fd;
  } else {
    throw RunTimeError(RunTimeError::ErrorCode::FILE,
                       boost::format("%1%: %2%")% file_name% strerror(errno));
  }
}

int AppendFile(std::string file_name) {
  int fd = open(file_name.c_str(), O_CREAT | O_WRONLY | O_APPEND,
                S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR | S_IROTH);

  if (fd > 0) {
    return fd;
  } else {
    throw RunTimeError(RunTimeError::ErrorCode::FILE,
                       boost::format("%1%: %2%")% file_name% strerror(errno));
  }
}

int ReadFile(std::string file_name) {
  int fd = open(file_name.c_str(), O_RDONLY,
                S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR | S_IROTH);

  if (fd > 0) {
    return fd;
  } else {
    throw RunTimeError(RunTimeError::ErrorCode::FILE,
                       boost::format("%1%: %2%")% file_name% strerror(errno));
  }
}

int Var2Pipe(std::string var, SymbolTableStack& sym_tab) {
  ObjectPtr obj = sym_tab.Lookup(var, false).SharedAccess();

  if (obj->type() != Object::ObjectType::STRING) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type: %1% has no cmd interface")%
                       static_cast<TypeObject&>(*obj->ObjType()).name());
  }

  const int READ = 0;
  const int WRITE = 1;

  int pipettes[2];

  pipe(pipettes);

  const std::string& str = static_cast<StringObject&>(*obj).value();
  const char* buf = str.c_str();

  write(pipettes[WRITE], buf, str.length());
  close(pipettes[WRITE]);

  return pipettes[READ];
}

}
}
