#include "glob.h"

#include "glob-cpp/file-glob.h"

namespace shpp {
namespace internal {

std::string GetGlobStr(Glob* glob) {
  std::vector<AstNode*> pieces = glob->children();
  std::string str_glob = "";

  // assign the glob to string
  for (AstNode* piece: pieces) {
    CmdPiece* part = static_cast<CmdPiece*>(piece);

    str_glob += part->cmd_str();

    if (part->blank_after()) {
      str_glob += " ";
    }
  }

  return str_glob;
}

std::vector<ObjectPtr> ExecGlob(const std::string& glob_str,
    SymbolTableStack& symbol_table_stack) {
  namespace fs = boost::filesystem;

  ObjectFactory obj_factory(symbol_table_stack);
  std::vector<ObjectPtr> glob_obj;

  glob::file_glob fglob{glob_str};
  std::vector<glob::path_match> results = fglob.Exec();

  for (auto& res : results) {
    ObjectPtr path_obj = obj_factory.NewPath(res.path());
    std::vector<ObjectPtr> sub_str_match;
    auto& match_res = res.match_result();
    for (auto& token : match_res) {
      ObjectPtr str_token_obj = obj_factory.NewString(token);
      sub_str_match.push_back(str_token_obj);
    }

    ObjectPtr array_token_obj = obj_factory.NewArray(std::move(sub_str_match));
    ObjectPtr tuple_obj = obj_factory.NewTuple(
        std::vector<ObjectPtr>{path_obj, array_token_obj});
    glob_obj.push_back(tuple_obj);
  }

  return glob_obj;
}

std::vector<std::string> GlobArguments(const std::string& arg) {
  std::vector<std::string> arg_vec;

  glob::file_glob fglob{arg};
  std::vector<glob::path_match> results = fglob.Exec();

  for (auto& res : results) {
    std::string str_path = res.path().string();
    arg_vec.push_back(std::move(str_path));
  }

  if (arg_vec.empty()) {
    arg_vec.push_back(arg);
  }

  return arg_vec;
}

}
}
