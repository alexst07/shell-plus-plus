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

ObjectPtr FuncDeclExecutor::FuncObj(AstNode* node) {
  FunctionDeclaration* fdecl_node = static_cast<FunctionDeclaration*>(node);
  auto vec = fdecl_node->children();
  size_t variadic_count = 0;
  std::vector<std::string> param_names;
  std::vector<ObjectPtr> default_values;

  // if the method is declared inside class
  // insert the parameter this
  if (method_) {
    param_names.push_back(std::string("this"));
  }

  for (FunctionParam* param: vec) {
    if (param->variadic()) {
      variadic_count++;
    }

    param_names.push_back(param->id()->name());
  }

  // only the last parameter can be variadic
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

  return fobj;
}

void FuncDeclExecutor::Exec(AstNode* node) {
  FunctionDeclaration* fdecl_node = static_cast<FunctionDeclaration*>(node);

  ObjectPtr fobj(FuncObj(node));

  // global symbol
  SymbolAttr entry(fobj, true);

  symbol_table_stack().InsertEntry(fdecl_node->name()->name(),
                                   std::move(entry));
}

void FuncDeclExecutor::set_stop(StopFlag flag) {
  parent()->set_stop(flag);
}

void ClassDeclExecutor::Exec(AstNode* node) {
  ClassDeclaration* class_decl_node = static_cast<ClassDeclaration*>(node);

  // handle class block
  ClassBlock* block = class_decl_node->block();
  ClassDeclList* decl_list = block->decl_list();

  ObjectPtr type_obj = obj_factory_.NewDeclType(
        class_decl_node->name()->name());

  // insert all declared methods on symbol table
  std::vector<Declaration*> decl_vec = decl_list->children();

  // the last argument specify that is a method inside the class
  FuncDeclExecutor fexec(this, symbol_table_stack(), true);

  for (auto decl: decl_vec) {
    if (decl->type() == AstNode::NodeType::kFunctionDeclaration) {
      // insert method on symbol table of class
      FunctionDeclaration* fdecl = static_cast<FunctionDeclaration*>(decl);

      // handle no abstract method
      if (fdecl->has_block()) {
        static_cast<TypeObject&>(*type_obj).RegiterMethod(fdecl->name()->name(),
                                                          fexec.FuncObj(decl));
      }
    } else if (decl->type() == AstNode::NodeType::kClassDeclaration) {
      ClassDeclaration* class_decl = static_cast<ClassDeclaration*>(decl);

      // insert inner class on type_obj symbol table, insted of its own
      ClassDeclExecutor class_exec(this, static_cast<DeclClassType&>(
          *type_obj).SymTableStack());
      class_exec.Exec(class_decl);
    }
  }

  SymbolAttr symbol_obj(type_obj, true);
  symbol_table_stack().InsertEntry(class_decl_node->name()->name(),
                                   std::move(symbol_obj));
}

void ClassDeclExecutor::set_stop(StopFlag flag) {
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

    case AstNode::NodeType::kWhileStatement: {
      WhileExecutor while_executor(this, symbol_table_stack());
      while_executor.Exec(static_cast<WhileStatement*>(node));
    } break;

    case AstNode::NodeType::kClassDeclaration: {
      ClassDeclExecutor class_decl_executor(this, symbol_table_stack());
      class_decl_executor.Exec(static_cast<ClassDeclaration*>(node));
      break;
    }
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

    // if vret there is only one element, return this element
    if (vret.size() == 1) {
      symbol_table_stack().SetEntryOnFunc("%return", vret[0]);
    } else {
      // convert vector to tuple object and insert it on symbol table
      // with reserved name
      ObjectPtr tuple_obj(obj_factory_.NewTuple(std::move(vret)));
      symbol_table_stack().SetEntryOnFunc("%return", tuple_obj);
    }
  } else {
    // return null
    ObjectPtr null_obj(obj_factory_.NewNull());
    symbol_table_stack().SetEntryOnFunc("%return", null_obj);
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

  // create a new table for if else scope
  symbol_table_stack().NewTable();

  BlockExecutor block_exec(this, symbol_table_stack());

  if (cond) {
    block_exec.Exec(node->then_block());
  } else {
    if (node->has_else()) {
      block_exec.Exec(node->else_block());
    }
  }

  // remove the scope
  symbol_table_stack().Pop();
}

void IfElseExecutor::set_stop(StopFlag flag) {
  parent()->set_stop(flag);
}

void WhileExecutor::Exec(WhileStatement* node) {
  // Executes if expresion
  ExpressionExecutor expr_exec(this, symbol_table_stack());

  auto fn_exp = [&expr_exec](Expression* exp)-> bool {
    ObjectPtr obj_exp = expr_exec.Exec(exp);
    bool cond = static_cast<BoolObject&>(*obj_exp->ObjBool()).value();
    return cond;
  };

  // create a new table for while scope
  symbol_table_stack().NewTable();

  BlockExecutor block_exec(this, symbol_table_stack());

  while (fn_exp(node->exp())) {
    block_exec.Exec(node->block());
  }

  // remove the scope
  symbol_table_stack().Pop();
}

void WhileExecutor::set_stop(StopFlag flag) {
  parent()->set_stop(flag);
}

}
}
