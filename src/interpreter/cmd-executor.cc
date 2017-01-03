#include "cmd-executor.h"

#include <unistd.h>
#include <unistd.h>
#include <fcntl.h>
#include <climits>
#include <cstdio>

namespace setti {
namespace internal {

std::tuple<int, std::string> CmdExecutor::ExecGetResult(CmdFull *node) {
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

std::tuple<int, std::string> CmdExecutor::ExecSimpleCmdWithResult(
    SimpleCmd *node) {
  SimpleCmdExecutor simple_cmd(this, symbol_table_stack());
  std::vector<std::string> cmd_args =
      simple_cmd.Exec(node);

  // status will be 0 if the command is executed on background
  int status = 0;

  // str_out will be empty if the command is executed on background
  std::string str_out = "";

  const int READ = 0;
  const int WRITE = 1;

  int pipettes[2];

  pipe(pipettes);

  fcntl(pipettes[READ], F_SETFL, fcntl(pipettes[READ], F_GETFL) | O_NONBLOCK);

  pid_t pid;
  pid = fork();

  if (pid == 0) {  
    close(pipettes[READ]);
    dup2(pipettes[WRITE], STDOUT_FILENO);
    ExecCmd(std::move(cmd_args));
  }

  status = WaitCmd(pid);
  close(pipettes[WRITE]);

  char buf[PIPE_BUF];
  int rd = 0;


  rd = read(pipettes[READ], buf, PIPE_BUF);

  if (rd > 0) {
    buf[rd] = '\0';
    str_out += buf;
  }

  close(pipettes[READ]);

  return std::tuple<int, std::string>(status, str_out);
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

  return str_part;
}

Job CmdIoRedirectListExecutor::PrepareData(CmdIoRedirectList *node) {
  // starts job struct
  Job job;
  job.shell_is_interactive_ = 0;
  job.stderr_ = STDERR_FILENO;
  job.stdout_ = STDOUT_FILENO;
  job.stdin_ = STDIN_FILENO;

  // iterate over redirect io list
  std::vector<CmdIoRedirect*> cmd_io_list = node->children();
  for (auto& l : cmd_io_list) {
    int fd;

    std::string file_name = FileName(l->file_path_cmd());
    std::cout << "file name: " << file_name << "\n";

    if (l->kind() == TokenKind::GREATER_THAN) {
      fd = open(file_name.c_str(), O_CREAT | O_WRONLY | O_TRUNC,
                S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR | S_IROTH);
    } else if (l->kind() == TokenKind::SAR) {
      fd = open(file_name.c_str(), O_CREAT | O_WRONLY | O_APPEND,
                S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR | S_IROTH);
    } else if (l->kind() == TokenKind::LESS_THAN) {
      fd = open(file_name.c_str(), O_RDONLY,
                S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR | S_IROTH);
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

  return job;
}

int CmdIoRedirectListExecutor::Exec(CmdIoRedirectList *node, bool background) {
  Job job = PrepareData(node);
  job.LaunchJob(!background);

  return job.Status();
}

std::tuple<int, std::string> CmdIoRedirectListExecutor::Exec(
    CmdIoRedirectList *node) {
//  CmdIoRedirectData cmd_io_redirect = PrepareData(node);
//  return ExecCmdIoWithResult(std::move(cmd_io_redirect));
}

////////////////////////////////////////////////////////////////////////////////
void CmdPipeSequenceExecutor::InputFile(CmdIoRedirectList* file, Job& job) {
  for (auto& io: file->children()) {
    std::string file_name =
        CmdIoRedirectListExecutor::FileName(io->file_path_cmd());

    int fd;
    // get only the input file
    if (io->kind() == TokenKind::LESS_THAN) {
      fd = open(file_name.c_str(), O_RDONLY,
                S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR | S_IROTH);
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
      fd = open(file_name.c_str(), O_CREAT | O_WRONLY | O_TRUNC,
                S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR | S_IROTH);
      SelectInterface(io, job, fd);
    } else if (io->kind() == TokenKind::SAR) {
      fd = open(file_name.c_str(), O_CREAT | O_WRONLY | O_APPEND,
                S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR | S_IROTH);
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

}
}
