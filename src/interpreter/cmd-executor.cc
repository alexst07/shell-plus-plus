#include "cmd-executor.h"

namespace setti {
namespace internal {

int CmdExecutor::Exec(CmdFull *node) {
  switch (node->cmd()->type()) {
    case AstNode::NodeType::kSimpleCmd: {
      SimpleCmdExecutor simple_cmd(this, symbol_table_stack());

      std::vector<std::string> cmd_str =
          simple_cmd.Exec(static_cast<SimpleCmd*>(node->cmd()));

      for (auto c: cmd_str) {
        std::cout << c << "|";
      }
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

std::tuple<std::string, int> CmdIoRedirectExecutor::Exec(CmdIoRedirect *node) {
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

  return std::tuple<std::string, int>(str_part, out);
}

}
}
