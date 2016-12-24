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
      return ExecSimpleCmd(static_cast<SimpleCmd*>(node->cmd()), background);
    } break;

    case AstNode::NodeType::kCmdIoRedirectList: {
      CmdIoRedirectListExecutor cmd_io(this, symbol_table_stack());
      cmd_io.Exec(static_cast<CmdIoRedirectList*>(node->cmd()), background);
    } break;

    default: {
      throw RunTimeError(RunTimeError::ErrorCode::INVALID_OPCODE,
                         boost::format("invalid command ast"));
    }
  }
}

void CmdExecutor::ExecSimpleCmd(SimpleCmd *node, bool background) {
  SimpleCmdExecutor simple_cmd(this, symbol_table_stack());

  std::vector<std::string> cmd_args = simple_cmd.Exec(node);

  pid_t pid;
  pid = fork();

  if (pid == 0) {
    ExecCmd(std::move(cmd_args));
  }

  if (!background) {
    WaitCmd(pid);
  }
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

CmdIoData CmdIoRedirectExecutor::Exec(CmdIoRedirect *node) {
  FilePathCmd* file_path = node->file_path_cmd();
  std::vector<AstNode*> pieces = file_path->children();
  std::string str_part = "";

  int out = 0;

  if (node->has_integer()) {
    if (node->integer()->literal_type()) {
      out = boost::get<int>(node->integer()->value());
    }
  }

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

  CmdIoData cmd_io;
  cmd_io.content_ = str_part;
  cmd_io.all_ = node->all();
  cmd_io.n_iface_ = out;
  cmd_io.in_out_ = SelectDirection(node->kind());

  return cmd_io;
}

CmdIoData::Direction CmdIoRedirectExecutor::SelectDirection(TokenKind kind) {
  switch (kind) {
    case TokenKind::SHL:
    case TokenKind::LESS_THAN:
      return CmdIoData::Direction::IN;
      break;

    case TokenKind::GREATER_THAN:
      return CmdIoData::Direction::OUT;
      break;

    case TokenKind::SAR:
      return CmdIoData::Direction::OUT_APPEND;
      break;

    case TokenKind::SSHL:
      return CmdIoData::Direction::IN_VARIABLE;
      break;

    case TokenKind::SSAR:
      return CmdIoData::Direction::OUT_VARIABLE;
      break;

   default:
      return CmdIoData::Direction::OUT;
  }
}

////////////////////////////////////////////////////////////////////////////////

CmdIoRedirectData CmdIoRedirectListExecutor::PrepareData(
    CmdIoRedirectList *node) {
  std::vector<CmdIoRedirect*> cmd_io_list = node->children();
  CmdIoListData cmd_io_ls_data;

  CmdIoRedirectExecutor cmd_io_exec(this, symbol_table_stack());

  // get list of files for input or output
  for (CmdIoRedirect* cmd_io: cmd_io_list) {
    cmd_io_ls_data.push_back(std::move(cmd_io_exec.Exec(cmd_io)));
  }

  CmdIoRedirectData cmd_io_redirect;
  cmd_io_redirect.io_list_ = std::move(cmd_io_ls_data);

  switch (node->cmd()->type()) {
    case AstNode::NodeType::kSimpleCmd: {
      SimpleCmdExecutor simple_cmd_exec(this, symbol_table_stack());
      cmd_io_redirect.cmd_ =
          simple_cmd_exec.Exec(static_cast<SimpleCmd*>(node->cmd()));
    } break;

    default: {
      throw RunTimeError(RunTimeError::ErrorCode::INVALID_OPCODE,
                         boost::format("invalid command ast"));
    }
  }

  return cmd_io_redirect;
}

int CmdIoRedirectListExecutor::Exec(CmdIoRedirectList *node, bool background) {
  CmdIoRedirectData cmd_io_redirect = PrepareData(node);
  return ExecCmdIo(std::move(cmd_io_redirect), background);
}

std::tuple<int, std::string> CmdIoRedirectListExecutor::Exec(
    CmdIoRedirectList *node) {
  CmdIoRedirectData cmd_io_redirect = PrepareData(node);
  return ExecCmdIoWithResult(std::move(cmd_io_redirect));
}

int CmdIoRedirectListExecutor::SelectFile(CmdIoData::Direction direction,
                                          const std::string& file_name) {
  int fd;

  if (direction == CmdIoData::Direction::OUT) {
    fd = open(file_name.c_str(), O_CREAT | O_WRONLY | O_TRUNC,
              S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR | S_IROTH);
  } else if (direction == CmdIoData::Direction::OUT_APPEND) {
    fd = open(file_name.c_str(), O_CREAT | O_WRONLY | O_APPEND,
              S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR | S_IROTH);
  } else if (direction == CmdIoData::Direction::IN) {
    fd = open(file_name.c_str(), O_RDONLY,
              S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR | S_IROTH);
  }

  return fd;
}

void CmdIoRedirectListExecutor::CopyStdIo(int fd,
                                          CmdIoData::Direction direction,
                                          int iface,
                                          bool all) {
  if (direction == CmdIoData::Direction::OUT ||
      direction == CmdIoData::Direction::OUT_APPEND) {
    if (all) {
      dup2(fd, STDOUT_FILENO);
      dup2(fd, STDERR_FILENO);
    } else {
      if (iface == 0 || iface == 1) {
        dup2(fd, STDOUT_FILENO);
      } else if (iface == 2) {
        dup2(fd, STDERR_FILENO);
      }
    }
  } else if (direction == CmdIoData::Direction::IN) {
    dup2(fd, STDIN_FILENO);
  }
}

int CmdIoRedirectListExecutor::ExecCmdIo(CmdIoRedirectData&& io_data,
                                          bool background) {
  pid_t pid;
  pid = fork();
  std::vector<int> fds;
  int status = 0;

  if (pid == 0) {
    for (auto& l: io_data.io_list_) {
      std::string str_file = boost::get<std::string>(l.content_);

      int fd = SelectFile(l.in_out_, str_file);

      fds.push_back(fd);

      CopyStdIo(fd, l.in_out_, l.n_iface_, l.all_);
    }

    ExecCmd(std::move(io_data.cmd_));
  }

  if (!background) {
    status = WaitCmd(pid);
  }

  return status;
}

std::tuple<int, std::string> CmdIoRedirectListExecutor::ExecCmdIoWithResult(
    CmdIoRedirectData&& io_data) {
  std::vector<int> fds;
  int status = 0;
  std::string str_out = "";

  const int READ = 0;
  const int WRITE = 1;

  int pipettes[2];

  fcntl(pipettes[READ], F_SETFL, fcntl(pipettes[READ], F_GETFL) | O_NONBLOCK);

  pipe(pipettes);

  pid_t pid;
  pid = fork();

  if (pid == 0) {
    close(pipettes[READ]);
    dup2(pipettes[WRITE], STDOUT_FILENO);

    for (auto& l: io_data.io_list_) {
      std::string str_file = boost::get<std::string>(l.content_);

      int fd = SelectFile(l.in_out_, str_file);

      fds.push_back(fd);

      CopyStdIo(fd, l.in_out_, l.n_iface_, l.all_);
    }

    ExecCmd(std::move(io_data.cmd_));
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

//CmdPipeListData CmdPipeSequenceExecutor::Exec(CmdPipeSequence *node) {

//}

}
}
