#include "stmt_executor.h"

#include <string>
#include <iostream>
#include <boost/variant.hpp>

#include "assign_executor.h"
#include "expr_executor.h"
#include "func_object.h"

namespace setti {
namespace internal {

void StmtListExecutor::Exec(AstNode* node) {
  StatementList* stmt_list = static_cast<StatementList*>(node);
  StmtExecutor stmt_exec(this, symbol_table_stack());

  for (AstNode* stmt: stmt_list->children()) {
    stmt_exec.Exec(stmt);
  }
}

void FuncDeclExecutor::Exec(AstNode* node) {
  FunctionDeclaration* fdecl_node = static_cast<FunctionDeclaration*>(node);
  auto vec = fdecl_node->children();
  size_t variadic_count = 0;
  std::vector<std::string> param_names;
  std::vector<ObjectPtr> default_values;

  for (FunctionParam* param: vec) {
    if (param->variadic()) {
      variadic_count++;
    }

    param_names.push_back(param->id()->name());
  }

  if ((variadic_count > 1) ||
      (variadic_count == 1) && (!fdecl_node->variadic())) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("only last parameter can be variadic"));
  }

  SymbolTableStack st_stack = symbol_table_stack();

  ObjectPtr fobj(new FuncDeclObject(
      fdecl_node->name()->name(), fdecl_node->block(), std::move(st_stack),
      std::move(param_names), std::move(default_values),
      fdecl_node->variadic()));

  symbol_table_stack().SetEntry(fdecl_node->name()->name(), fobj);
}

void StmtExecutor::Exec(AstNode* node) {
  switch (node->type()) {
    case AstNode::NodeType::kAssignmentStatement: {
      AssignExecutor exec(this, symbol_table_stack());
      return exec.Exec(node);
    } break;

    case AstNode::NodeType::kExpressionStatement: {
      ExpressionExecutor exec(this, symbol_table_stack());
      exec.Exec(static_cast<ExpressionStatement&>(*node).exp());
      return;
    } break;

    case AstNode::NodeType::kFunctionDeclaration: {
      FuncDeclExecutor fdecl_executor(this, symbol_table_stack());
      fdecl_executor.Exec(node);
    } break;

  }
}

}
}
