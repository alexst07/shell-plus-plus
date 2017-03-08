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

#include <glob.h>

#include <string>
#include <boost/variant.hpp>

#include "cmd-executor.h"
#include "stmt-executor.h"

namespace shpp {
namespace internal {

std::vector<ObjectPtr> AssignableListExecutor::Exec(
    AstNode* node) {
  AssignableList* assign_list_node = static_cast<AssignableList*>(node);

  std::vector<ObjectPtr> obj_vec;

  for (AstNode* value: assign_list_node->children()) {
    obj_vec.push_back(ExecAssignable(value));
  }

  return obj_vec;
}

ObjectPtr AssignableListExecutor::ExecAssignable(AstNode* node) {
  AssignableValue* assignable_node = static_cast<AssignableValue*>(node);
  AstNode* value = assignable_node->value();

  if (value->type() == AstNode::NodeType::kFunctionDeclaration) {
    return ExecLambdaFunc(value);
  } else if (AstNode::IsExpression(value->type())) {
    ExpressionExecutor expr_exec(this, symbol_table_stack());
    return expr_exec.Exec(value);
  }

  throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                     boost::format("incompatible expression on assignable"),
                     node->pos());
}

ObjectPtr AssignableListExecutor::ExecLambdaFunc(AstNode* node) {
  // executes lambda assignment
  FuncDeclExecutor func_exec(this, symbol_table_stack(), false, true);
  return func_exec.FuncObj(node);
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

    default:
      throw RunTimeError(RunTimeError::ErrorCode::INVALID_OPCODE,
                         boost::format("invalid expression opcode"),
                         node->pos());
  }
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
      throw RunTimeError(e.err_code(), e.msg(), node->pos());
    }

    return array_obj;
  } else {
    std::vector<ObjectPtr> vec;
    std::shared_ptr<Object> array_obj;

    try {
      array_obj = obj_factory_.NewArray(std::move(vec));
    } catch (RunTimeError& e) {
      throw RunTimeError(e.err_code(), e.msg(), node->pos());
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
    throw RunTimeError(e.err_code(), e.msg(), node->pos());
  }

  return map;
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
  throw RunTimeError(e.err_code(), e.msg(), node->pos());
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
    throw RunTimeError(e.err_code(), e.msg(), array_node->index_exp()->pos());
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
  throw RunTimeError(e.err_code(), e.msg(), node->pos());
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
  throw RunTimeError(e.err_code(), e.msg(), node->pos());
}

ObjectPtr ExpressionExecutor::ExecNotExpr(AstNode* node) {
  if (node->type() == AstNode::NodeType::kNotExpression) {
    NotExpression *not_expr = static_cast<NotExpression*>(node);
    ObjectPtr exp(Exec(not_expr->exp()));

    try {
      return exp->Not();
    } catch (RunTimeError& e) {
      throw RunTimeError(e.err_code(), e.msg(), node->pos());
    }
  }

  UnaryOperation* not_expr = static_cast<UnaryOperation*>(node);
  ObjectPtr exp(Exec(not_expr->exp()));

  try {
    return exp->Not();
  } catch (RunTimeError& e) {
    throw RunTimeError(e.err_code(), e.msg(), node->pos());
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
    throw RunTimeError(e.err_code(), e.msg(), node->pos());
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

      default:
        throw RunTimeError(RunTimeError::ErrorCode::INVALID_OPCODE,
                           boost::format("invalid bin operation opcode"));
    }
  } catch (RunTimeError& e) {
    throw RunTimeError(e.err_code(), e.msg(), node->pos());
  }

  return res;
}

ObjectPtr ExpressionExecutor::ExecAttribute(Attribute* node) {
  // pass reference for any object
  ObjectPtr exp = Exec(node->exp(), true);
  std::string name = node->id()->name();

  try {
    return exp->Attr(exp, name);
  } catch (RunTimeError& e) {
    throw RunTimeError(e.err_code(), e.msg(), node->pos());
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
    throw RunTimeError(e.err_code(), e.msg(), node->pos());
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
    throw RunTimeError(e.err_code(), e.msg(), node->pos());
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
  std::vector<Expression*> expr_vec = expr_list_node->children();
  for (AstNode* value: expr_vec) {
    obj_vec.push_back(expr_executor.Exec(value));
  }

  return obj_vec;
}

void ExprListExecutor::set_stop(StopFlag flag) {
  parent()->set_stop(flag);
}

ObjectPtr FuncCallExecutor::Exec(FunctionCall* node) {
  ExpressionExecutor expr_exec(this, symbol_table_stack());
  ObjectPtr fobj = expr_exec.Exec(node->func_exp());

  AssignableListExecutor assignable_list(this, symbol_table_stack());
  auto vec = assignable_list.Exec(node->rvalue_list());

  switch (fobj->type()) {
    case Object::ObjectType::FUNC: {
      return static_cast<FuncObject&>(*fobj).Call(this, std::move(vec));
      break;
    }

    case Object::ObjectType::TYPE: {
      return static_cast<TypeObject&>(*fobj).Constructor(this, std::move(vec));
      break;
    }

    default: {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("object is not callable"),
                         node->func_exp()->pos());
    }
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

std::string GlobExecutor::GetGlobStr(Glob* glob) {
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

ObjectPtr GlobExecutor::Exec(Glob* glob_node) {
  ObjectFactory obj_factory(symbol_table_stack());
  std::string glob_str = GetGlobStr(glob_node);

  if (glob_node->recursive()) {
    std::vector<ObjectPtr> glob_obj =
        ListTree(boost::filesystem::current_path(), glob_str);
    return obj_factory.NewArray(std::move(glob_obj));
  }

  std::vector<ObjectPtr> glob_obj = ExecGlob(glob_str);
  return obj_factory.NewArray(std::move(glob_obj));
}

boost::filesystem::recursive_directory_iterator GlobExecutor::CreateRIterator(
    boost::filesystem::path path) try {
  return boost::filesystem::recursive_directory_iterator(path);
} catch(boost::filesystem::filesystem_error& fex) {
  return boost::filesystem::recursive_directory_iterator();
}

std::vector<ObjectPtr> GlobExecutor::ExecDir(boost::filesystem::path path,
    const std::string& glob_str, const std::string& root) try {
  std::vector<ObjectPtr> vec;

  if (boost::filesystem::is_directory(path)) {
    // change the current path to calculate the glob, after that return
    // to the preview path
    boost::filesystem::path pwd(boost::filesystem::current_path());
    current_path(path);
    vec = ExecGlob(glob_str, root);
    current_path(pwd);
  }

  return vec;
} catch(boost::filesystem::filesystem_error& e) {
  throw RunTimeError(RunTimeError::ErrorCode::GLOB,
                     boost::format("%1%")% e.what());
}

std::vector<ObjectPtr> GlobExecutor::ListTree(boost::filesystem::path path,
    const std::string& glob_str) {
  std::vector<ObjectPtr> vec = ExecDir(path, glob_str);

  boost::filesystem::recursive_directory_iterator it = CreateRIterator(path);
  boost::filesystem::recursive_directory_iterator end;

  while(it != end) {
    // when iterating over sub directories pass path root string, to calculate
    // relative path
    std::vector<ObjectPtr> temp_vec = ExecDir(*it, glob_str, path.string());
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

std::vector<ObjectPtr> GlobExecutor::ExecGlob(const std::string& glob_str,
    const std::string& root_str) {
  namespace fs = boost::filesystem;

  ObjectFactory obj_factory(symbol_table_stack());
  std::vector<ObjectPtr> glob_obj;

  glob_t globbuf;
  int flag = GLOB_NOMAGIC | GLOB_MARK | GLOB_BRACE | GLOB_TILDE;

  glob(glob_str.c_str(), flag, NULL, &globbuf);

  for (int i = 0; i < globbuf.gl_pathc; i++) {
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

}
}
