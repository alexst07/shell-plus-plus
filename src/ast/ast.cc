#include "ast.h"

namespace shpp {
namespace internal {

static const char* ast_node_str[] = {
  #define DECLARE_TYPE_CLASS(type) #type,
    AST_NODE_LIST(DECLARE_TYPE_CLASS)
  #undef DECLARE_TYPE_CLASS
  ""
};

// define function only to avoid warning
const char* AstNodeStr(size_t i) {
  return ast_node_str[i];
}

}
}
