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
    if (stop_flag_ == StopFlag::kGo) {
      stmt_exec.Exec(stmt);
    } else {
      return;
    }
  }
}

void StmtListExecutor::set_stop(StopFlag flag) {
  parent()->set_stop(flag);
  stop_flag_ = flag;
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

  SymbolTableStack st_stack(symbol_table_stack());

  ObjectPtr fobj(obj_factory_.NewFuncDeclObject(fdecl_node->name()->name(),
                                                fdecl_node->block(),
                                                std::move(st_stack),
                                                std::move(param_names),
                                                std::move(default_values),
                                                fdecl_node->variadic()));

  symbol_table_stack().SetEntry(fdecl_node->name()->name(), fobj);
}

void FuncDeclExecutor::set_stop(StopFlag flag) {
  parent()->set_stop(flag);
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

    case AstNode::NodeType::kReturnStatement: {
      ReturnExecutor ret_executor(this, symbol_table_stack());
      ret_executor.Exec(node);
    } break;

    case AstNode::NodeType::kIfStatement: {
      IfElseExecutor ifelse_executor(this, symbol_table_stack());
      ifelse_executor.Exec(static_cast<IfStatement*>(node));
    } break;

  }
}

void StmtExecutor::set_stop(StopFlag flag) {
  parent()->set_stop(flag);
}

void ReturnExecutor::Exec(AstNode* node) {
  ReturnStatement* ret_node = static_cast<ReturnStatement*>(node);

  if (!ret_node->is_void()) {
    AssignableListExecutor assign_list(this, symbol_table_stack());
    std::vector<ObjectPtr> vret = assign_list.Exec(ret_node->assign_list());

    // convert vector to tuple object and insert it on symbol table
    // with reserved name
    ObjectPtr tuple_obj(obj_factory_.NewTuple(std::move(vret)));
    symbol_table_stack().SetEntry("%return", tuple_obj);
  } else {
    // return null
    ObjectPtr null_obj(obj_factory_.NewNull());
    symbol_table_stack().SetEntry("%return", null_obj);
  }

  // set stop return
  parent()->set_stop(StopFlag::kReturn);
}

void ReturnExecutor::set_stop(StopFlag flag) {
  parent()->set_stop(flag);
}

void IfElseExecutor::Exec(IfStatement* node) {
  // Executes if expresion
  ExpressionExecutor expr_exec(this, symbol_table_stack());
  ObjectPtr obj_exp = expr_exec.Exec(node->exp());
  bool cond = static_cast<BoolObject&>(*obj_exp->ObjBool()).value();

  BlockExecutor block_exec(this, symbol_table_stack());

  if (cond) {
    block_exec.Exec(node->then_block());
  } else {
    if (node->has_else()) {
      block_exec.Exec(node->else_block());
    }
  }
}

void IfElseExecutor::set_stop(StopFlag flag) {
  parent()->set_stop(flag);
}

}
}
