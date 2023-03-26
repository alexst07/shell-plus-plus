#include "ast.h"

namespace shpp {
namespace internal {

static const char* ast_node_str[] = {
#define DECLARE_TYPE_CLASS(type) #type,
    AST_NODE_LIST(DECLARE_TYPE_CLASS)
#undef DECLARE_TYPE_CLASS
        ""};

// define function only to avoid warning
const char* AstNodeStr(size_t i) { return ast_node_str[i]; }

AnnotationDeclaration::AnnotationDeclaration(
    std::unique_ptr<Expression> decorator_expr,
    std::unique_ptr<Declaration> decl, Position position)
    : Declaration(NodeType::kCmdDeclaration, position),
      decorator_expr_(std::move(decorator_expr)),
      decl_(std::move(decl)) {
  if (decl_->type() == NodeType::kFunctionDeclaration) {
    std::string name =
        reinterpret_cast<FunctionDeclaration*>(decl_.get())->name()->name();
    original_name_ = name;
    reinterpret_cast<FunctionDeclaration*>(decl_.get())
        ->SetName(std::string("@") + name);
  } else if (decl_->type() == NodeType::kClassDeclaration) {
    std::string name =
        reinterpret_cast<ClassDeclaration*>(decl_.get())->name()->name();
    original_name_ = name;
    reinterpret_cast<ClassDeclaration*>(decl_.get())
        ->SetName(std::string("@") + name);
  }
}

}  // namespace internal
}  // namespace shpp
