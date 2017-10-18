#include "glob.h"

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

boost::filesystem::recursive_directory_iterator CreateRIterator(
    boost::filesystem::path path) try {
  return boost::filesystem::recursive_directory_iterator(path);
} catch(boost::filesystem::filesystem_error& fex) {
  return boost::filesystem::recursive_directory_iterator();
}

std::vector<ObjectPtr> ExecDir(boost::filesystem::path path,
    const std::string& glob_str, SymbolTableStack& symbol_table_stack,
    const std::string& root) try {
  std::vector<ObjectPtr> vec;

  if (boost::filesystem::is_directory(path)) {
    // change the current path to calculate the glob, after that return
    // to the preview path
    boost::filesystem::path pwd(boost::filesystem::current_path());
    current_path(path);
    vec = ExecGlob(glob_str, symbol_table_stack, root);
    current_path(pwd);
  }

  return vec;
} catch(boost::filesystem::filesystem_error& e) {
  throw RunTimeError(RunTimeError::ErrorCode::GLOB,
                     boost::format("%1%")% e.what());
}

std::vector<ObjectPtr> ListTree(boost::filesystem::path path,
    const std::string& glob_str, SymbolTableStack& symbol_table_stack) {
  std::vector<ObjectPtr> vec = ExecDir(path, glob_str, symbol_table_stack);

  boost::filesystem::recursive_directory_iterator it = CreateRIterator(path);
  boost::filesystem::recursive_directory_iterator end;

  while(it != end) {
    // when iterating over sub directories pass path root string, to calculate
    // relative path
    std::vector<ObjectPtr> temp_vec = ExecDir(*it, glob_str, symbol_table_stack,
        path.string());
    vec.insert(vec.end(), temp_vec.begin(), temp_vec.end());

    if(boost::filesystem::is_directory(*it) &&
       boost::filesystem::is_symlink(*it)) {
      it.no_push();
    }

    try {
      ++it;
    } catch(std::exception& e) {
      it.no_push();
      throw RunTimeError(RunTimeError::ErrorCode::GLOB,
                         boost::format("%1%")% e.what());
    }
  }

  return vec;
}

std::vector<ObjectPtr> ExecGlob(const std::string& glob_str,
    SymbolTableStack& symbol_table_stack, const std::string& root_str) {
  namespace fs = boost::filesystem;

  ObjectFactory obj_factory(symbol_table_stack);
  std::vector<ObjectPtr> glob_obj;

  glob_t globbuf;
  int flag = GLOB_NOMAGIC | GLOB_MARK | GLOB_BRACE | GLOB_TILDE;

  glob(glob_str.c_str(), flag, NULL, &globbuf);

  for (int i = 0; i < static_cast<int>(globbuf.gl_pathc); i++) {
    // calculates the relative directory, the easy way is to execute substr
    // since all others directories a sub directory it doesn't need to
    // calculate complex relative path
    if (!root_str.empty()) {
      std::string path = boost::filesystem::current_path().string();

      // add 1 to eleminate the character '/' that separates path
      path = path.substr(root_str.length() + 1);

      std::string str_glob_res = std::string(globbuf.gl_pathv[i]);
      std::string p = (fs::path(path) / fs::path(str_glob_res)).string();

      ObjectPtr str_obj = obj_factory.NewString(p);
      glob_obj.push_back(str_obj);
      continue;
    }

    ObjectPtr str_obj = obj_factory.NewString(std::string(globbuf.gl_pathv[i]));
    glob_obj.push_back(str_obj);
  }

  globfree(&globbuf);

  return glob_obj;
}

std::vector<std::string> GlobArguments(const std::string& arg) {
  glob_t globbuf;
  std::vector<std::string> arg_vec;

  int flag = GLOB_NOMAGIC | GLOB_BRACE | GLOB_TILDE;

  glob(arg.c_str(), flag, nullptr, &globbuf);

  for (size_t i = 0; i < globbuf.gl_pathc; i++) {
    arg_vec.push_back(std::string(globbuf.gl_pathv[i]));
  }

  // free glob
  globfree (&globbuf);

  if (arg_vec.size() == 0) {
    return std::vector<std::string>(1, arg);
  }

  return arg_vec;
}

}
}
