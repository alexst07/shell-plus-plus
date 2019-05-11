#ifndef SHPP_GLOB_H
#define SHPP_GLOB_H

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
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

std::vector<ObjectPtr> ExecGlob(const std::string& glob_str,
    SymbolTableStack& symbol_table_stack);

std::vector<std::string> GlobArguments(const std::string& arg);

}
}

#endif  // SHPP_GLOB_H
