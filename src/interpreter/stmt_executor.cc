#include "stmt_executor.h"

#include <string>
#include <iostream>
#include <boost/variant.hpp>

#include "scope-executor.h"
#include "assign_executor.h"
#include "expr_executor.h"
#include "func_object.h"
#include "cmd-executor.h"
#include "utils/scope-exit.h"

namespace setti {
namespace internal {

void StmtListExecutor::Exec(AstNode* node) {
  StatementList* stmt_list = static_cast<StatementList*>(node);
  StmtExecutor stmt_exec(this, symbol_table_stack());

  for (AstNode* stmt: stmt_list->children()) {
    // when stop flag is set inside some control struct of function
    // it don't pass ahead this point, because this struct must
    // set parent only if it will not use the flag
    // for example: loops must not set this flag for continue and
    // break, but must set this flag for return and throw
    if (stop_flag_ == StopFlag::kGo) {
      stmt_exec.Exec(stmt);
    } else {
      return;
    }
  }
}

void StmtListExecutor::set_stop(StopFlag flag) {
  stop_flag_ = flag;

  if (parent() == nullptr) {
    return;
  }

  parent()->set_stop(flag);
}

ObjectPtr FuncDeclExecutor::FuncObj(AstNode* node) {
  FunctionDeclaration* fdecl_node = static_cast<FunctionDeclaration*>(node);
  auto vec = fdecl_node->children();
  size_t variadic_count = 0;
  std::vector<std::string> param_names;
  std::vector<ObjectPtr> default_values;

  // if the method is declared inside of a class
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
  if (parent() == nullptr) {
    return;
  }

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
  if (parent() == nullptr) {
    return;
  }

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

    case AstNode::NodeType::kForInStatement: {
      ForInExecutor for_executor(this, symbol_table_stack());
      for_executor.Exec(static_cast<ForInStatement*>(node));
    } break;

    case AstNode::NodeType::kClassDeclaration: {
      ClassDeclExecutor class_decl_executor(this, symbol_table_stack());
      class_decl_executor.Exec(static_cast<ClassDeclaration*>(node));
    } break;

    case AstNode::NodeType::kBreakStatement: {
      BreakExecutor break_executor(this, symbol_table_stack());
      break_executor.Exec(static_cast<BreakStatement*>(node));
    } break;

    case AstNode::NodeType::kContinueStatement: {
      ContinueExecutor continue_executor(this, symbol_table_stack());
      continue_executor.Exec(static_cast<ContinueStatement*>(node));
    } break;

    case AstNode::NodeType::kCmdFull: {
      CmdExecutor cmd_full(this, symbol_table_stack());
      cmd_full.Exec(static_cast<CmdFull*>(node));
    } break;

    case AstNode::NodeType::kSwitchStatement: {
      SwitchExecutor switch_stmt(this, symbol_table_stack());
      switch_stmt.Exec(static_cast<SwitchStatement*>(node));
    } break;

    case AstNode::NodeType::kDeferStatement: {
      DeferExecutor defer(this, symbol_table_stack());
      defer.Exec(static_cast<DeferStatement*>(node));
    }
  }
}

void StmtExecutor::set_stop(StopFlag flag) {
  if (parent() == nullptr) {
    return;
  }

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

  auto fn_exp = [&](Expression* exp)-> bool {
    // if break was called or throw or return must exit from loop
    if (stop_flag_ == StopFlag::kBreak || stop_flag_ == StopFlag::kThrow ||
        stop_flag_ == StopFlag::kReturn) {
      return false;
    }

    ObjectPtr obj_exp = expr_exec.Exec(exp);
    bool cond = static_cast<BoolObject&>(*obj_exp->ObjBool()).value();
    return cond;
  };

  // create a new table for while scope
  symbol_table_stack().NewTable();

  // scope exit case an excpetion thrown
  auto cleanup = MakeScopeExit([&]() {
    // remove the scope
    symbol_table_stack().Pop();
  });
  IgnoreUnused(cleanup);

  BlockExecutor block_exec(this, symbol_table_stack());

  while (fn_exp(node->exp())) {
    block_exec.Exec(node->block());
  }
}

void WhileExecutor::set_stop(StopFlag flag) {
  stop_flag_ = flag;

  if (parent() == nullptr) {
    return;
  }

  if (flag != StopFlag::kBreak && flag != StopFlag::kContinue) {
    parent()->set_stop(flag);
  }
}

void ForInExecutor::Assign(std::vector<std::reference_wrapper<ObjectPtr>>& vars,
                           std::vector<ObjectPtr>& it_values) {
  ObjectFactory obj_factory(symbol_table_stack());

  // Assignment can be done only when the tuples have the same size
  // or there is only one variable on the test side
  // a, b, c = 1, 2, 3; a = 1, 2, 3; a, b = f
  if ((vars.size() != 1) && (it_values.size() != 1) &&
      (vars.size() != it_values.size())) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("different size of tuples"));
  }

  if ((vars.size() == 1) && (it_values.size() == 1)) {
    vars[0].get() = it_values[0]->Next();
  } else if ((vars.size() == 1) && (it_values.size() != 1)) {
    std::vector<ObjectPtr> values;

    // Call next method from every iterator
    for (auto& v: it_values) {
      values.push_back(v->Next());
    }

    ObjectPtr tuple_obj(obj_factory.NewTuple(std::move(values)));
    vars[0].get() = tuple_obj;
  } else if ((vars.size() != 1) && (it_values.size() == 1)) {
    // get iterator point element
    ObjectPtr obj_ptr = it_values[0]->Next();

    // only tuple object is accept on this case
    if (obj_ptr->type() != Object::ObjectType::TUPLE) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("expect tuple object as test"));
    }

    // numbers of variables must be equal the number of tuple elements
    TupleObject &tuple_obj = static_cast<TupleObject&>(*obj_ptr);
    if (vars.size() != tuple_obj.Size()) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
          boost::format("numbers of variables: %1% and "
                        "numbers of tuples: %2% are "
                        "incompatibles")% vars.size() % tuple_obj.Size());
    }

    for (size_t i = 0; i < vars.size(); i++) {
      vars[i].get() = tuple_obj.Element(i);
    }
  } else {
    // on this case there are the same number of variables and values
    for (size_t i = 0; i < vars.size(); i++) {
      vars[i].get() = it_values[i]->Next();
    }
  }
}

void ForInExecutor::Exec(ForInStatement* node) {
  // create a new table for while scope
  symbol_table_stack().NewTable();

  // scope exit case an excpetion thrown
  auto cleanup = MakeScopeExit([&]() {
    // remove the scope
    symbol_table_stack().Pop();
  });
  IgnoreUnused(cleanup);

  AssignExecutor assign_exec(this, symbol_table_stack());

  // Executes the left side of for statemente
  auto vars = assign_exec.AssignList(node->exp_list());

  // Executes the test side of for statemente
  ExprListExecutor expr_list(this, symbol_table_stack());
  auto containers = expr_list.Exec(node->test_list());

  std::vector<ObjectPtr> it_values;

  // get iterator of each object
  for (auto& it: containers) {
    ObjectPtr obj(it->ObjIter(it));
    it_values.push_back(obj);
  }

  auto fn_exp = [&]()-> bool {
    // if break was called or throw or return must exit from loop
    if (stop_flag_ == StopFlag::kBreak || stop_flag_ == StopFlag::kThrow ||
        stop_flag_ == StopFlag::kReturn) {
      return false;
    }

    // check if all items on it_values has next
    for (auto& it: it_values) {
      // as it is a reference, change the pointer inside it_values
      ObjectPtr has_next_obj = it->HasNext();

      if (has_next_obj->type() != Object::ObjectType::BOOL) {
        throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                           boost::format("expect bool from __has_next__"));
      }

      bool v = static_cast<BoolObject&>(*has_next_obj).value();
      if (!v) {
        return false;
      }
    }

    // assign the it_values->Next to vars references to be used inside
    // block of for statemente
    Assign(vars, it_values);

    return true;
  };

  BlockExecutor block_exec(this, symbol_table_stack());

  while (fn_exp()) {
    block_exec.Exec(node->block());
  }
}

void ForInExecutor::set_stop(StopFlag flag) {
  stop_flag_ = flag;

  if (parent() == nullptr) {
    return;
  }

  if (flag != StopFlag::kBreak && flag != StopFlag::kContinue) {
    parent()->set_stop(flag);
  }
}

void BreakExecutor::Exec(BreakStatement* /*node*/) {
  // set stop break
  parent()->set_stop(StopFlag::kBreak);
}

void BreakExecutor::set_stop(StopFlag flag) {
  parent()->set_stop(flag);
}

void ContinueExecutor::Exec(ContinueStatement* /*node*/) {
  // set stop break
  parent()->set_stop(StopFlag::kContinue);
}

void ContinueExecutor::set_stop(StopFlag flag) {
  parent()->set_stop(flag);
}

bool SwitchExecutor::MatchAnyExp(ObjectPtr exp,
                                 std::vector<ObjectPtr> &&exp_list) {
  // compare each expression from list with exp
  for (auto& e: exp_list) {
    ObjectPtr res(exp->Equal(e));

    if (res->type() != Object::ObjectType::BOOL) {
      continue;
    }

    BoolObject& obj_test = static_cast<BoolObject&>(*res);

    if (obj_test.value()) {
      return true;
    }
  }

  return false;
}

void SwitchExecutor::Exec(SwitchStatement* node) {
  BlockExecutor block_exec(this, symbol_table_stack());
  ExpressionExecutor expr_exec(this, symbol_table_stack());
  ObjectPtr obj_exp_switch;

  if (node->has_exp()) {
     obj_exp_switch = expr_exec.Exec(node->exp());
  } else {
    // if the switch doesn't have expression, compare with true
    ObjectFactory obj_factory(symbol_table_stack());
    obj_exp_switch = obj_factory.NewBool(true);
  }

  // flag to mark if any case statement was executed
  bool any_case_executed = false;

  // executes each case and compare each expression
  std::vector<CaseStatement*> case_list = node->case_list();

  for (auto& c: case_list) {
    ExprListExecutor expr_list_exec(this, symbol_table_stack());
    std::vector<ObjectPtr> obj_res_list = expr_list_exec.Exec(c->exp_list());

    // compare each expression with switch expression
    if (MatchAnyExp(obj_exp_switch, std::move(obj_res_list))) {
      any_case_executed = true;

      // create a new table for while scope
      symbol_table_stack().NewTable();

      // scope exit case an excpetion thrown
      auto cleanup = MakeScopeExit([&]() {
        // remove the scope
        symbol_table_stack().Pop();
      });
      IgnoreUnused(cleanup);

      // if any expression match with case expression, executes case's block
      block_exec.Exec(c->block());
    }
  }

  // if any case was not executed, so execute the default clause
  // if switch statement has one
  if (!any_case_executed && node->has_default()) {
    block_exec.Exec(node->default_stmt()->block());
  }
}

void SwitchExecutor::set_stop(StopFlag flag) {
  parent()->set_stop(flag);
}

void DeferExecutor::Exec(DeferStatement *node) {
  // push the statement on main block parent
  Executor* exec = GetMainExecutor();

  if (exec != nullptr) {
    static_cast<ScopeExecutor*>(exec)->PushDeferStmt(node->stmt());
  }
}

void DeferExecutor::set_stop(StopFlag flag) {
  parent()->set_stop(flag);
}

}
}
