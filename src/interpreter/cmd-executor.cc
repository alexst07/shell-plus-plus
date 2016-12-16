#include "cmd-executor.h"

namespace setti {
namespace internal {

int CmdExecutor::Exec(CmdFull *node) {
  CmdData cmd_data;

  switch (node->cmd()->type()) {
    case AstNode::NodeType::kSimpleCmd: {
      SimpleCmdExecutor simple_cmd(this, symbol_table_stack());
      cmd_data = simple_cmd.Exec(static_cast<SimpleCmd*>(node->cmd()));
    } break;

    case AstNode::NodeType::kCmdIoRedirectList: {
      CmdIoRedirectListExecutor cmd_io(this, symbol_table_stack());
      cmd_data = cmd_io.Exec(static_cast<CmdIoRedirectList*>(node->cmd()));
    } break;
  }
}

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
    }
  }

  // if the cmd doesn't finish with blank space, put its content on vector
  if (!blank_after && is_cmd_piece) {
    cmd.push_back(str_part);
  }

  return cmd;
}

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

CmdIoRedirectData CmdIoRedirectListExecutor::Exec(CmdIoRedirectList *node) {
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
  }

  return cmd_io_redirect;
}

CmdPipeListData CmdPipeSequenceExecutor::Exec(CmdPipeSequence *node) {

}

}
}
