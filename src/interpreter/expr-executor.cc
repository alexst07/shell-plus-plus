// Copyright 2016 Alex Silva Torres
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "expr-executor.h"

#include <string>
#include <boost/variant.hpp>

#include "assign-executor.h"
#include "cmd-executor.h"
#include "stmt-executor.h"
#include "utils/glob.h"
#include "utils/scope-exit.h"

namespace shpp {
namespace internal {

std::vector<ObjectPtr> AssignableListExecutor::Exec(
    AstNode* node) {
  AssignableList* assign_list_node = static_cast<AssignableList*>(node);

  std::vector<ObjectPtr> obj_vec;
  EllipsisExprExecutor ellipsis_expr_executor(this, symbol_table_stack());

  for (AstNode* value: assign_list_node->children()) {
    if (value->type() == AstNode::NodeType::kEllipsisExpression) {
      std::vector<ObjectPtr> elems = ellipsis_expr_executor.Exec(value);
      obj_vec.insert(obj_vec.end(), elems.begin(), elems.end());
    } else {
      obj_vec.push_back(ExecAssignable(value));
    }
  }

  return obj_vec;
}

ObjectPtr AssignableListExecutor::ExecAssignable(AstNode* node) {
  AssignableValue* assignable_node = static_cast<AssignableValue*>(node);
  AstNode* value = assignable_node->value();

  if (AstNode::IsExpression(value->type())) {
    ExpressionExecutor expr_exec(this, symbol_table_stack());
    return expr_exec.Exec(value);
  }

  throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                     boost::format("incompatible expression on assignable"),
                     node->pos());
}

void AssignableListExecutor::set_stop(StopFlag flag) {
  parent()->set_stop(flag);
}

ObjectPtr ExpressionExecutor::Exec(AstNode* node, bool pass_ref) {
  pass_ref_ = pass_ref;

  switch (node->type()) {
    case AstNode::NodeType::kLiteral: {
      return ExecLiteral(node);
    } break;

    case AstNode::NodeType::kIdentifier: {
      return ExecIdentifier(node);
    } break;

    case AstNode::NodeType::kArray: {
      return ExecArrayAccess(node);
    } break;

    case AstNode::NodeType::kArrayInstantiation: {
      return ExecArrayInstantiation(node);
    } break;

    case AstNode::NodeType::kDictionaryInstantiation: {
      return ExecMapInstantiation(node);
    } break;

    case AstNode::NodeType::kFunctionCall: {
      return ExecFuncCall(static_cast<FunctionCall*>(node));
    } break;

    case AstNode::NodeType::kBinaryOperation:
      return ExecBinOp(static_cast<BinaryOperation*>(node));
      break;

    case AstNode::NodeType::kAttribute:
      return ExecAttribute(static_cast<Attribute*>(node));
      break;

    case AstNode::NodeType::kCmdExpression:
      return ExecCmdExpr(static_cast<CmdExpression*>(node));
      break;

    case AstNode::NodeType::kSlice:
      return ExecSlice(static_cast<Slice*>(node));
      break;

    case AstNode::NodeType::kNotExpression:
      return ExecNotExpr(static_cast<NotExpression*>(node));
      break;

    case AstNode::NodeType::kUnaryOperation:
      return ExecUnary(static_cast<UnaryOperation*>(node));
      break;

    case AstNode::NodeType::kNullExpression:
      return ExecNull();
      break;

    case AstNode::NodeType::kGlob:
      return ExecGlob(static_cast<Glob*>(node));
      break;

    case AstNode::NodeType::kFunctionExpression:
      return ExecLambdaFunc(node);
      break;

    case AstNode::NodeType::kLetExpression:
      return ExecLetExpression(static_cast<LetExpression*>(node));
      break;

    case AstNode::NodeType::kListComprehension:
      return ExecListComprehension(node);
      break;

    case AstNode::NodeType::kIfElseExpression:
      return ExecIfElseExpr(node);
      break;

    default:
      throw RunTimeError(RunTimeError::ErrorCode::INVALID_OPCODE,
                         boost::format("invalid expression opcode"),
                         node->pos());
  }
}

ObjectPtr ExpressionExecutor::ExecLambdaFunc(AstNode* node) {
  // executes lambda assignment
  FuncDeclExecutor func_exec(this, symbol_table_stack(), false, true);
  return func_exec.FuncObj(node);
}

ObjectPtr ExpressionExecutor::ExecArrayInstantiation(AstNode* node) {
  ArrayInstantiation* array_node = static_cast<ArrayInstantiation*>(node);
  AssignableListExecutor assignable_list(this, symbol_table_stack());

  if (array_node->has_elements()) {
    auto vec = assignable_list.Exec(array_node->assignable_list());
    std::shared_ptr<Object> array_obj;

    try {
      array_obj = obj_factory_.NewArray(std::move(vec));
    } catch (RunTimeError& e) {
      throw RunTimeError(e.err_code(), e.msg(), node->pos(), e.messages());
    }

    return array_obj;
  } else {
    std::vector<ObjectPtr> vec;
    std::shared_ptr<Object> array_obj;

    try {
      array_obj = obj_factory_.NewArray(std::move(vec));
    } catch (RunTimeError& e) {
      throw RunTimeError(e.err_code(), e.msg(), node->pos(), e.messages());
    }

    return array_obj;
  }
}

ObjectPtr ExpressionExecutor::ExecMapInstantiation(AstNode* node) {
  DictionaryInstantiation* map_node =
      static_cast<DictionaryInstantiation*>(node);

  std::vector<std::pair<ObjectPtr, ObjectPtr>> map_vec;
  auto children_vec = map_node->children();

  // traverses the vector assembling the vector of pairs of objects
  for (auto& key_value: children_vec) {
    ObjectPtr obj_key(Exec(key_value->key()));
    AssignableListExecutor assignable(this, symbol_table_stack());
    ObjectPtr obj_value(assignable.ExecAssignable(key_value->value()));
    std::pair<ObjectPtr, ObjectPtr> pair(obj_key, obj_value);
    map_vec.push_back(std::move(pair));
  }

  ObjectPtr map;

  try {
    // creates the map object
    map = obj_factory_.NewMap(std::move(map_vec));
  } catch (RunTimeError& e) {
    throw RunTimeError(e.err_code(), e.msg(), node->pos(), e.messages());
  }

  return map;
}

ObjectPtr ExpressionExecutor::ExecLetExpression(LetExpression* node) {
  AssignExecutor exec(this, symbol_table_stack());
  auto p = exec.ExecWithReturn(node->assign());
  return p;
}

ObjectPtr ExpressionExecutor::ExecIdentifier(AstNode* node) try {
  Identifier* id_node = static_cast<Identifier*>(node);
  const std::string& name = id_node->name();
  auto obj = symbol_table_stack().Lookup(name, false).Ref();

  if (pass_ref_) {
    return obj;
  } else {
    return PassVar(obj, symbol_table_stack());
  }
} catch (RunTimeError& e) {
  throw RunTimeError(e.err_code(), e.msg(), node->pos(), e.messages());
}

ObjectPtr ExpressionExecutor::ExecGlob(Glob* glob) {
  GlobExecutor glob_exec(this, symbol_table_stack());
  return glob_exec.Exec(glob);
}

ObjectPtr ExpressionExecutor::ExecArrayAccess(AstNode* node) {
  Array* array_node = static_cast<Array*>(node);
  Expression* arr_exp = array_node->arr_exp();

  ObjectPtr array_obj = Exec(arr_exp);

  // Executes index expression of array
  ObjectPtr index = Exec(array_node->index_exp());

  ObjectPtr val;
  try {
    val = array_obj->GetItem(index);
  } catch (RunTimeError& e) {
    throw RunTimeError(e.err_code(), e.msg(), array_node->index_exp()->pos(),
        e.messages());
  }

  if (pass_ref_) {
    return val;
  } else {
    return PassVar(val, symbol_table_stack());
  }
}

ObjectPtr ExpressionExecutor::ExecFuncCall(FunctionCall* node)
try {
  FuncCallExecutor fcall_exec(this, symbol_table_stack());
  return fcall_exec.Exec(node);
} catch (RunTimeError& e) {
  e.messages().Push(std::move(Message(Message::Severity::ERR,
      boost::format("on function call"),
      node->pos().line, node->pos().col)));

  // when the exception is thrown inside some object function
  // this object doesn't have the position of the node, so
  // we have to check if the line is 0 to set the correct line
  // on this case
  Position pos = {e.pos().line == 0? node->pos().line:e.pos().line,
      e.pos().col == 0? node->pos().col:e.pos().col};
  throw RunTimeError(e.err_code(), e.msg(), pos, e.messages());
}

ObjectPtr ExpressionExecutor::ExecLiteral(AstNode* node) try {
  Literal* literal = static_cast<Literal*>(node);
  switch (literal->literal_type()) {
    case Literal::Type::kInteger: {
      ObjectPtr obj(obj_factory_.NewInt(boost::get<int>(literal->value())));
      return obj;
    } break;

    case Literal::Type::kBool: {
      ObjectPtr obj(obj_factory_.NewBool(boost::get<bool>(literal->value())));
      return obj;
    } break;

    case Literal::Type::kReal: {
      ObjectPtr obj(obj_factory_.NewReal(boost::get<float>(literal->value())));
      return obj;
    } break;

    case Literal::Type::kString: {
    std::string str = boost::get<std::string>(literal->value());
      ObjectPtr obj(obj_factory_.NewString(std::move(str)));
      return obj;
    } break;
  }
} catch (RunTimeError& e) {
  throw RunTimeError(e.err_code(), e.msg(), node->pos(), e.messages());
}

ObjectPtr ExpressionExecutor::ExecNotExpr(AstNode* node) {
  if (node->type() == AstNode::NodeType::kNotExpression) {
    NotExpression *not_expr = static_cast<NotExpression*>(node);
    ObjectPtr exp(Exec(not_expr->exp()));

    try {
      return exp->Not();
    } catch (RunTimeError& e) {
      throw RunTimeError(e.err_code(), e.msg(), node->pos(), e.messages());
    }
  }

  UnaryOperation* not_expr = static_cast<UnaryOperation*>(node);
  ObjectPtr exp(Exec(not_expr->exp()));

  try {
    return exp->Not();
  } catch (RunTimeError& e) {
    throw RunTimeError(e.err_code(), e.msg(), node->pos(), e.messages());
  }
}

ObjectPtr ExpressionExecutor::ExecNull() {
  return obj_factory_.NewNull();
}

ObjectPtr ExpressionExecutor::ExecUnary(AstNode* node) {
  UnaryOperation* unary_expr = static_cast<UnaryOperation*>(node);
  ObjectPtr unary_obj = Exec(static_cast<AstNode*>(unary_expr->exp()));

  try {
    switch (unary_expr->kind()) {
      case TokenKind::ADD:
        return unary_obj->UnaryAdd();
        break;

      case TokenKind::SUB:
        return unary_obj->UnarySub();
        break;

      case TokenKind::EXCL_NOT:
        return ExecNotExpr(unary_expr);
        break;

      case TokenKind::BIT_NOT:
        return unary_obj->BitNot();
        break;

      default:
        throw RunTimeError(RunTimeError::ErrorCode::INVALID_OPCODE,
                           boost::format("invalid unary operation opcode"));
    }
  } catch (RunTimeError& e) {
    throw RunTimeError(e.err_code(), e.msg(), node->pos(), e.messages());
  }
}

ObjectPtr ExpressionExecutor::ExecBinOp(BinaryOperation* node) {
  // Executes the left and right side of the expression
  ObjectPtr left = Exec(static_cast<AstNode*>(node->left()));
  ObjectPtr right = Exec(static_cast<AstNode*>(node->right()));

  ObjectPtr res;

  try {
    switch (node->kind()) {
      case TokenKind::ADD:
        res = left->Add(right);
        break;

      case TokenKind::SUB:
        res = left->Sub(right);
        break;

      case TokenKind::MUL:
        res = left->Mult(right);
        break;

      case TokenKind::DIV:
        res = left->Div(right);
        break;

      case TokenKind::MOD:
        res = left->DivMod(right);
        break;

      case TokenKind::SAR:
        res = left->RightShift(right);
        break;

      case TokenKind::SHL:
        res = left->LeftShift(right);
        break;

      case TokenKind::BIT_AND:
        res = left->BitAnd(right);
        break;

      case TokenKind::BIT_OR:
        res = left->BitOr(right);
        break;

      case TokenKind::BIT_XOR:
        res = left->BitXor(right);
        break;

      case TokenKind::AND:
        res = left->And(right);
        break;

      case TokenKind::OR:
        res = left->Or(right);
        break;

      case TokenKind::EQUAL:
        res = left->Equal(right);
        break;

      case TokenKind::NOT_EQUAL:
        res = left->NotEqual(right);
        break;

      case TokenKind::LESS_THAN:
        res = left->Lesser(right);
        break;

      case TokenKind::GREATER_THAN:
        res = left->Greater(right);
        break;

      case TokenKind::LESS_EQ:
        res = left->LessEqual(right);
        break;

      case TokenKind::GREATER_EQ:
        res = left->GreatEqual(right);
        break;

      case TokenKind::KW_IN:
        res = right->In(left);
        break;

      case TokenKind::KW_INSTANCEOF:
        res = ExecInstanceOf(left, right);
        break;

      case TokenKind::KW_IS:
        res = ExecIs(left, right);
        break;

      default:
        throw RunTimeError(RunTimeError::ErrorCode::INVALID_OPCODE,
                           boost::format("invalid bin operation opcode"));
    }
  } catch (RunTimeError& e) {
    throw RunTimeError(e.err_code(), e.msg(), node->pos(), e.messages());
  }

  return res;
}

ObjectPtr ExpressionExecutor::ExecIs(ObjectPtr obj, ObjectPtr type) {
  bool res = obj->ObjType().get() == type.get();
  return obj_factory_.NewBool(res);
}

ObjectPtr ExpressionExecutor::ExecInstanceOf(ObjectPtr obj, ObjectPtr base) {
  bool res = InstanceOf(obj, base);
  return obj_factory_.NewBool(res);
}

ObjectPtr ExpressionExecutor::ExecAttribute(Attribute* node) {
  // pass reference for any object
  ObjectPtr exp = Exec(node->exp(), true);
  std::string name = node->id()->name();

  try {
    return exp->Attr(exp, name);
  } catch (RunTimeError& e) {
    throw RunTimeError(e.err_code(), e.msg(), node->pos(), e.messages());
  }
}

ObjectPtr ExpressionExecutor::ExecSlice(Slice* node) {
  ObjectPtr start(obj_factory_.NewNull());
  ObjectPtr end(obj_factory_.NewNull());

  // TODO: in the future the slice expression must support step x:y:z
  ObjectPtr step(obj_factory_.NewInt(1));

  if (node->has_start_exp()) {
    start = Exec(node->start_exp());
  }

  if (node->has_end_exp()) {
    end = Exec(node->end_exp());
  }

  try {
    return obj_factory_.NewSlice(start, end, step);
  } catch (RunTimeError& e) {
    throw RunTimeError(e.err_code(), e.msg(), node->pos(), e.messages());
  }
}

ObjectPtr ExpressionExecutor::ExecCmdExpr(CmdExpression* node) {
  CmdExecutor cmd_full(this, symbol_table_stack());
  CmdExprData res(cmd_full.ExecGetResult(
      static_cast<CmdFull*>(node->cmd())));

  // create command object
  int status = std::get<0>(res);
  std::string str = std::get<1>(res);
  ObjectFactory obj_factory(symbol_table_stack());

  try {
    ObjectPtr obj(obj_factory.NewCmdObj(status, std::move(str),
                                        std::move(std::string(""))));
    return obj;
  } catch (RunTimeError& e) {
    throw RunTimeError(e.err_code(), e.msg(), node->pos(), e.messages());
  }
}

ObjectPtr ExpressionExecutor::ExecListComprehension(AstNode* node) {
  ListComprehensionExecutor list_comp(this, symbol_table_stack());
  return list_comp.Exec(node);
}

ObjectPtr ExpressionExecutor::ExecIfElseExpr(AstNode* node) {
  IfElseExpression* if_else_expr = static_cast<IfElseExpression*>(node);

  // Executes if expresion
  ExpressionExecutor expr_exec(this, symbol_table_stack());
  ObjectPtr obj_exp = expr_exec.Exec(if_else_expr->exp());

  bool cond;

  try {
    cond = static_cast<BoolObject&>(*obj_exp->ObjBool()).value();
  } catch (RunTimeError& e) {
    throw RunTimeError(e.err_code(), e.msg(), if_else_expr->exp()->pos(),
        e.messages());
  }

  if (cond) {
    return expr_exec.Exec(if_else_expr->then_exp());
  } else {
    return expr_exec.Exec(if_else_expr->else_exp());
  }
}

void ExpressionExecutor::set_stop(StopFlag flag) {
  parent()->set_stop(flag);
}

std::vector<ObjectPtr> ExprListExecutor::Exec(
    AstNode* node) {
  ExpressionList* expr_list_node = static_cast<ExpressionList*>(node);

  std::vector<ObjectPtr> obj_vec;

  ExpressionExecutor expr_executor(this, symbol_table_stack());
  EllipsisExprExecutor ellipsis_expr_executor(this, symbol_table_stack());

  std::vector<Expression*> expr_vec = expr_list_node->children();
  for (AstNode* value: expr_vec) {
    if (value->type() == AstNode::NodeType::kEllipsisExpression) {
      std::vector<ObjectPtr> elems = ellipsis_expr_executor.Exec(value);
      obj_vec.insert(obj_vec.end(), elems.begin(), elems.end());
    } else {
      obj_vec.push_back(expr_executor.Exec(value));
    }
  }

  return obj_vec;
}

void ExprListExecutor::set_stop(StopFlag flag) {
  parent()->set_stop(flag);
}

std::tuple<ArgumentsExecutor::Args, ArgumentsExecutor::KWArgs>
ArgumentsExecutor::Exec(ArgumentsList* args_list) {
  bool found_kwarg = false;
  Args args;
  KWArgs kw_args;

  AssignableListExecutor assign_exec(this, symbol_table_stack());

  for (Argument* arg: args_list->children()) {
    if (arg->has_key()) {
      found_kwarg = true;

      std::string name = arg->key();

      ObjectPtr value = assign_exec.ExecAssignable(arg->arg());

      kw_args.insert(std::pair<std::string, ObjectPtr>(name, value));
    } else {
      if (found_kwarg) {
        throw RunTimeError(RunTimeError::ErrorCode::INVALID_ARGS,
            boost::format("positional argument follows keyword argument"));
      }

      EllipsisExprExecutor ellipsis_expr_executor(this, symbol_table_stack());

      if (arg->arg()->type() == AstNode::NodeType::kEllipsisExpression) {
        std::vector<ObjectPtr> elems = ellipsis_expr_executor.Exec(arg->arg());
        args.insert(args.end(), elems.begin(), elems.end());
      } else {
        args.push_back(assign_exec.ExecAssignable(arg->arg()));
      }
    }
  }

  return std::tuple<Args, KWArgs>(std::move(args), std::move(kw_args));
}

void ArgumentsExecutor::set_stop(StopFlag flag) {
  parent()->set_stop(flag);
}

ObjectPtr FuncCallExecutor::Exec(FunctionCall* node) {
  ExpressionExecutor expr_exec(this, symbol_table_stack());
  ObjectPtr fobj = expr_exec.Exec(node->func_exp());

  ArgumentsExecutor args_list(this, symbol_table_stack());

  Args args;
  KWArgs kw_args;
  std::tie(args, kw_args) = args_list.Exec(node->args_list());

  try {
    switch (fobj->type()) {
      case Object::ObjectType::FUNC: {
        return static_cast<FuncObject&>(*fobj).Call(this, std::move(args),
            std::move(kw_args));
        break;
      }

      case Object::ObjectType::SPEC_FUNC: {
        return static_cast<SpecialFuncObject&>(*fobj).SpecialCall(this,
            std::move(args), std::move(kw_args), symbol_table_stack());
        break;
      }

      case Object::ObjectType::TYPE: {
        return static_cast<TypeObject&>(*fobj).Constructor(this,
            std::move(args), std::move(kw_args));
        break;
      }

      default: {
        return fobj->Call(this, std::move(args), std::move(kw_args));
      }
    }
  } catch (RunTimeError& e) {
    throw RunTimeError(e.err_code(), e.msg(), e.pos(), e.messages());
  }
}

void FuncCallExecutor::set_stop(StopFlag flag) {
  // only throw is passed to outside function
  // breaks, continues, are not allowed inside
  // functions, and return is consumed by
  // function
  if (flag == StopFlag::kThrow) {
    parent()->set_stop(flag);
  }
}

std::vector<ObjectPtr> EllipsisExprExecutor::Exec(AstNode* node) {
  EllipsisExpression* ellipsis_expr = static_cast<EllipsisExpression*>(node);
  Expression* expr = ellipsis_expr->expr();

  std::vector<ObjectPtr> obj_vec;

  ExpressionExecutor expr_executor(this, symbol_table_stack());
  ObjectPtr obj_expr = expr_executor.Exec(expr);

  // execute iterator from expression object, put each item on a vector
  // and this vector will be used to append expression list, so,
  // with this feature is possible unpack elements on function arguments
  ObjectPtr obj_iter = obj_expr->ObjIter(obj_expr);

  // check if there is next value on iterator
  auto check_iter = [&] () -> bool {
    ObjectPtr has_next_obj = obj_iter->HasNext();
    if (has_next_obj->type() != Object::ObjectType::BOOL) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("expect bool from __has_next__"),
                         node->pos());
    }

    bool v = static_cast<BoolObject&>(*has_next_obj).value();
    return v;
  };

  while (check_iter()) {
    ObjectPtr next_obj = obj_iter->Next();
    obj_vec.push_back(next_obj);
  }

  return obj_vec;
}

void EllipsisExprExecutor::set_stop(StopFlag flag) {
  // only throw is passed to outside function
  // breaks, continues, are not allowed inside
  // functions, and return is consumed by
  // function
  if (flag == StopFlag::kThrow) {
    parent()->set_stop(flag);
  }
}

ObjectPtr GlobExecutor::Exec(Glob* glob_node) {
  ObjectFactory obj_factory(symbol_table_stack());
  std::string glob_str = GetGlobStr(glob_node);

  if (glob_node->recursive()) {
    std::vector<ObjectPtr> glob_obj =
        ListTree(boost::filesystem::current_path(), glob_str,
        symbol_table_stack());

    return obj_factory.NewArray(std::move(glob_obj));
  }

  std::vector<ObjectPtr> glob_obj = ExecGlob(glob_str, symbol_table_stack());
  return obj_factory.NewArray(std::move(glob_obj));
}

ObjectPtr ListComprehensionExecutor::Exec(AstNode* node) {
  ListComprehension* list_comp_node = static_cast<ListComprehension*>(node);
  std::vector<Expression*> for_if_list = list_comp_node->comp_list();

  // create a new table for while scope
  symbol_table_stack().NewTable();
  ObjectPtr& ref = symbol_table_stack().Lookup("%list_comp_val", true).Ref();

  // scope exit case an excpetion thrown
  auto cleanup = MakeScopeExit([&]() {
    // remove the scope
    symbol_table_stack().Pop();
  });
  IgnoreUnused(cleanup);

  ObjectFactory obj_factory(symbol_table_stack());
  ref = obj_factory.NewArray(std::vector<std::unique_ptr<Object>>());

  // process the last operation
  Expression* last_item = for_if_list[for_if_list.size() - 1];

  std::unique_ptr<Statement> stmt;

  if (last_item->type() == AstNode::NodeType::kCompIf) {
    CompIf* comp_if = static_cast<CompIf*>(last_item);
    stmt = std::move(MountIfBlock(comp_if, list_comp_node));
  } else {
    CompFor* comp_for = static_cast<CompFor*>(last_item);
    stmt = std::move(MountForBlock(comp_for, list_comp_node));
  }

  AstNodeFactory ast_node_factory([&]() {
    return list_comp_node->pos();
  });

  if (for_if_list.size() > 1) {
    stmt = std::move(ExecForIfList(for_if_list, std::move(stmt),
        ast_node_factory));
  }

  StmtExecutor stmt_exec(this, symbol_table_stack());
  stmt_exec.Exec(stmt.get());

  return symbol_table_stack().Lookup("%list_comp_val", false).SharedAccess();
}

std::unique_ptr<Statement> ListComprehensionExecutor::ExecForIfList(
    std::vector<Expression*>& for_if_list, std::unique_ptr<Statement>&& stmt_l,
    AstNodeFactory& ast_node_factory) {
  std::unique_ptr<Statement> stmt = std::move(stmt_l);
  for (int i = for_if_list.size() - 2; i >= 0; i--) {
    Expression* item = for_if_list[i];

    std::vector<std::unique_ptr<Statement>> stmt_vec;
    stmt_vec.push_back(std::move(stmt));

    std::unique_ptr<StatementList> stmt_list(ast_node_factory.NewStatementList(
        std::move(stmt_vec)));

    auto block = ast_node_factory.NewBlock(std::move(stmt_list));

    if (item->type() == AstNode::NodeType::kCompIf) {
      CompIf* comp_if = static_cast<CompIf*>(item);

      std::unique_ptr<Statement> if_stmt(ast_node_factory.NewIfStatement(
          std::move(comp_if->MoveCompExp()), std::move(block),
          std::unique_ptr<Statement>(nullptr)));

      stmt = std::move(if_stmt);
    } else {
      CompFor* comp_for = static_cast<CompFor*>(item);

      std::unique_ptr<Statement> for_stmt(ast_node_factory.NewForInStatement(
          std::move(comp_for->MoveExpList()),
          std::move(comp_for->MoveTestList()), std::move(block)));

      stmt = std::move(for_stmt);
    }
  }

  return stmt;
}

std::unique_ptr<Statement> ListComprehensionExecutor::MountBlock(
    ListComprehension* list_comp_node) {
  AstNodeFactory ast_node_factory([&]() {
    return list_comp_node->pos();
  });

  std::unique_ptr<Identifier> id(ast_node_factory.NewIdentifier(
      "%list_comp_val", std::move(std::unique_ptr<PackageScope>(nullptr))));

  std::unique_ptr<Identifier> id_func_name(ast_node_factory.NewIdentifier(
      "append", std::move(std::unique_ptr<PackageScope>(nullptr))));

  std::unique_ptr<Attribute> attribute(ast_node_factory.NewAttribute(
      std::move(id), std::move(id_func_name)));

  std::unique_ptr<Argument> arg(ast_node_factory.NewArgument(
      std::string(""), std::move(list_comp_node->MoveResExp())));

  std::vector<std::unique_ptr<Argument>> arg_vec;
  arg_vec.push_back(std::move(arg));

  std::unique_ptr<ArgumentsList> arg_list(ast_node_factory.NewArgumentsList(
      std::move(arg_vec)));

  // mount function call
  std::unique_ptr<FunctionCall> func_call(ast_node_factory.NewFunctionCall(
      std::move(attribute), std::move(arg_list)));

  std::vector<std::unique_ptr<Statement>> stmt_vec;
  stmt_vec.push_back(std::move(func_call));

  std::unique_ptr<StatementList> stmt_list(ast_node_factory.NewStatementList(
      std::move(stmt_vec)));

  auto block = ast_node_factory.NewBlock(std::move(stmt_list));

  return block;
}

std::unique_ptr<Statement> ListComprehensionExecutor::MountIfBlock(
    CompIf* comp_if, ListComprehension* list_comp_node) {
  AstNodeFactory ast_node_factory([&]() {
    return list_comp_node->pos();
  });

  auto block = MountBlock(list_comp_node);

  // mount if statement
  std::unique_ptr<Statement> if_stmt(ast_node_factory.NewIfStatement(
      std::move(comp_if->MoveCompExp()), std::move(block),
      std::unique_ptr<Statement>(nullptr)));

  return if_stmt;
}

std::unique_ptr<Statement> ListComprehensionExecutor::MountForBlock(
    CompFor* comp_for, ListComprehension* list_comp_node) {
  AstNodeFactory ast_node_factory([&]() {
    return list_comp_node->pos();
  });

  auto block = MountBlock(list_comp_node);

  // mount if statement
  std::unique_ptr<Statement> for_stmt(ast_node_factory.NewForInStatement(
      std::move(comp_for->MoveExpList()), std::move(comp_for->MoveTestList()),
      std::move(block)));

  return for_stmt;
}

}
}
