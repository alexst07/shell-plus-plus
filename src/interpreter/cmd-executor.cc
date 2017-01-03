#include "cmd-executor.h"

#include <unistd.h>
#include <unistd.h>
#include <fcntl.h>
#include <climits>
#include <cstdio>
#include <boost/algorithm/string.hpp>

namespace setti {
namespace internal {

CmdExprData CmdExecutor::ExecGetResult(CmdFull *node) {
  CmdData cmd_data;

  switch (node->cmd()->type()) {
    case AstNode::NodeType::kSimpleCmd: {
      return ExecSimpleCmdWithResult(static_cast<SimpleCmd*>(node->cmd()));
    } break;

    case AstNode::NodeType::kCmdIoRedirectList: {
      CmdIoRedirectListExecutor cmd_io(this, symbol_table_stack());
      return cmd_io.Exec(static_cast<CmdIoRedirectList*>(node->cmd()));
    } break;

    default: {
      throw RunTimeError(RunTimeError::ErrorCode::INVALID_OPCODE,
                         boost::format("invalid command ast"));
    }
  }
}

void CmdExecutor::Exec(CmdFull *node) {
  bool background = node->background();

  switch (node->cmd()->type()) {
    case AstNode::NodeType::kSimpleCmd: {
      ExecSimpleCmd(static_cast<SimpleCmd*>(node->cmd()), background);
    } break;

    case AstNode::NodeType::kCmdIoRedirectList: {
      CmdIoRedirectListExecutor cmd_io(this, symbol_table_stack());
      cmd_io.Exec(static_cast<CmdIoRedirectList*>(node->cmd()), background);
    } break;

    case AstNode::NodeType::kCmdPipeSequence: {
      CmdPipeSequenceExecutor cmd_pipe(this, symbol_table_stack());
      cmd_pipe.Exec(static_cast<CmdPipeSequence*>(node->cmd()), background);
    } break;

    default: {
      throw RunTimeError(RunTimeError::ErrorCode::INVALID_OPCODE,
                         boost::format("invalid command ast"));
    }
  }
}

void CmdExecutor::ExecSimpleCmd(SimpleCmd *node, bool foreground) {
  SimpleCmdExecutor simple_cmd(this, symbol_table_stack());

  std::vector<std::string> cmd_args = simple_cmd.Exec(node);

  Job job;
  Process p(std::move(cmd_args));
  job.process_.push_back(p);
  job.shell_is_interactive_ = 0;
  job.stderr_ = STDERR_FILENO;
  job.stdout_ = STDOUT_FILENO;
  job.stdin_ = STDIN_FILENO;
  job.LaunchJob(foreground);
}

CmdExprData CmdExecutor::ExecSimpleCmdWithResult(
    SimpleCmd *node) {
  SimpleCmdExecutor simple_cmd(this, symbol_table_stack());

  std::vector<std::string> cmd_args = simple_cmd.Exec(node);

  const int READ = 0;
  const int WRITE = 1;

  int pipettes[2];

  pipe(pipettes);

  Job job;
  Process p(std::move(cmd_args));
  job.process_.push_back(p);
  job.shell_is_interactive_ = 0;
  job.stderr_ = STDERR_FILENO;
  job.stdout_ = pipettes[WRITE];
  job.stdin_ = STDIN_FILENO;
  job.LaunchJob(true);

  char buf[PIPE_BUF];
  int rd = 0;

  rd = read(pipettes[READ], buf, PIPE_BUF);

  std::string str_out = "";

  if (rd > 0) {
    buf[rd] = '\0';
    str_out += buf;
  }

  close(pipettes[READ]);

  return CmdExprData(job.Status(), str_out, "");
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
    switch (piece->type()) {
      case AstNode::NodeType::kCmdPiece: {
        is_cmd_piece = true;
        CmdPiece* cmd_part = static_cast<CmdPiece*>(piece);

        str_part += cmd_part->cmd_str();
        blank_after = cmd_part->blank_after();

        if (blank_after) {
          cmd.push_back(str_part);
          str_part = "";
        }
      } break;

      default: {
        throw RunTimeError(RunTimeError::ErrorCode::INVALID_OPCODE,
                           boost::format("invalid command ast"));
      }
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

std::string CmdIoRedirectListExecutor::FileName(FilePathCmd* file_path) {
  std::vector<AstNode*> pieces = file_path->children();
  std::string str_part = "";

  for (AstNode* piece: pieces) {
    switch (piece->type()) {
      case AstNode::NodeType::kCmdPiece: {
        CmdPiece* cmd_part = static_cast<CmdPiece*>(piece);

        str_part += cmd_part->cmd_str();

        if (cmd_part->blank_after()) {
          str_part += " ";
        }
      } break;

      default: {
        throw RunTimeError(RunTimeError::ErrorCode::INVALID_OPCODE,
                           boost::format("invalid command ast"));
      }
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

    std::string file_name = FileName(l->file_path_cmd());

    if (l->kind() == TokenKind::GREATER_THAN) {
      fd = CreateFile(file_name);
    } else if (l->kind() == TokenKind::SAR) {
      fd = AppendFile(file_name);
    } else if (l->kind() == TokenKind::LESS_THAN) {
      fd = ReadFile(file_name);
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

  Process p(std::move(cmd_args));
  job.process_.push_back(std::move(p));
}

int CmdIoRedirectListExecutor::Exec(CmdIoRedirectList *node, bool background) {
  // starts job struct
  Job job;
  job.shell_is_interactive_ = 0;
  job.stderr_ = STDERR_FILENO;
  job.stdout_ = STDOUT_FILENO;
  job.stdin_ = STDIN_FILENO;

  PrepareData(job, node);
  job.LaunchJob(!background);

  return job.Status();
}

CmdExprData CmdIoRedirectListExecutor::Exec(
    CmdIoRedirectList *node) {
  const int READ = 0;
  const int WRITE = 1;

  int pipettes[2];

  pipe(pipettes);

  Job job;
  job.shell_is_interactive_ = 0;
  job.stderr_ = STDERR_FILENO;
  job.stdout_ = pipettes[WRITE];
  job.stdin_ = STDIN_FILENO;

  PrepareData(job, node);
  job.LaunchJob(true);

  char buf[PIPE_BUF];
  int rd = 0;

  rd = read(pipettes[READ], buf, PIPE_BUF);

  std::string str_out = "";

  if (rd > 0) {
    buf[rd] = '\0';
    str_out += buf;
  }

  close(pipettes[READ]);

  return CmdExprData(job.Status(), str_out, "");
}

////////////////////////////////////////////////////////////////////////////////
void CmdPipeSequenceExecutor::InputFile(CmdIoRedirectList* file, Job& job) {
  for (auto& io: file->children()) {
    std::string file_name =
        CmdIoRedirectListExecutor::FileName(io->file_path_cmd());

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
        CmdIoRedirectListExecutor::FileName(io->file_path_cmd());

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

  Process p(std::move(cmd_args));
  job.process_.push_back(std::move(p));
}

int CmdPipeSequenceExecutor::Exec(CmdPipeSequence *node, bool background) {
  std::vector<Cmd*> cmds = node->cmds();

  Job job;
  job.shell_is_interactive_ = 0;
  job.stderr_ = STDERR_FILENO;
  job.stdout_ = STDOUT_FILENO;
  job.stdin_ = STDIN_FILENO;

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

  job.LaunchJob(!background);

  return job.Status();
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

}
}
