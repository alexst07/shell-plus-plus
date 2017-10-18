#ifndef SHPP_GLOB_H
#define SHPP_GLOB_H

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <glob.h>
#include <string>
#include <vector>
#include <memory>
#include <boost/filesystem.hpp>

#include "interpreter/symbol-table.h"
#include "ast/ast.h"
#include "objects/obj-type.h"
#include "interpreter/executor.h"
#include "objects/object-factory.h"

namespace shpp {
namespace internal {

std::string GetGlobStr(Glob* glob);

boost::filesystem::recursive_directory_iterator CreateRIterator(
    boost::filesystem::path path);

std::vector<ObjectPtr> ExecDir(boost::filesystem::path path,
    const std::string& glob_str, SymbolTableStack& symbol_table_stack,
    const std::string& root_str = "");

std::vector<ObjectPtr> ListTree(boost::filesystem::path path,
    const std::string& glob_str, SymbolTableStack& symbol_table_stack);

std::vector<ObjectPtr> ExecGlob(const std::string& glob_str,
    SymbolTableStack& symbol_table_stack, const std::string& root_str = "");

std::vector<std::string> GlobArguments(const std::string& arg);

}
}

#endif  // SHPP_GLOB_H
