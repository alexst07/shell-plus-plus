#include "expr_executor.h"

#include <string>
#include <boost/variant.hpp>

#include "cmd-executor.h"

namespace setti {
namespace internal {

std::vector<ObjectPtr> AssignableListExecutor::Exec(
    AstNode* node) {
  AssignableList* assign_list_node = static_cast<AssignableList*>(node);

  std::vector<ObjectPtr> obj_vec;

  for (AstNode* value: assign_list_node->children()) {
    obj_vec.push_back(std::move(ExecAssignable(value)));
  }

  return obj_vec;
}

ObjectPtr AssignableListExecutor::ExecAssignable(AstNode* node) {
  AssignableValue* assignable_node = static_cast<AssignableValue*>(node);
  if (AstNode::IsExpression(node->type())) {
    ExpressionExecutor expr_exec(this, symbol_table_stack());
    return expr_exec.Exec(assignable_node->value());
  }

  throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                     boost::format("incompatible expression on assignable"));
}

void AssignableListExecutor::set_stop(StopFlag flag) {
  parent()->set_stop(flag);
}

ObjectPtr ExpressionExecutor::Exec(AstNode* node) {
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

    default:
      throw RunTimeError(RunTimeError::ErrorCode::INVALID_OPCODE,
                         boost::format("invalid expression opcode"));
  }
}

ObjectPtr ExpressionExecutor::ExecArrayInstantiation(AstNode* node) {
  ArrayInstantiation* array_node = static_cast<ArrayInstantiation*>(node);
  AssignableListExecutor assignable_list(this, symbol_table_stack());
  auto vec = assignable_list.Exec(array_node->assignable_list());
  std::shared_ptr<Object> array_obj(obj_factory.NewArray(std::move(vec)));
  return array_obj;
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

  // creates the map object
  ObjectPtr map(obj_factory.NewMap(std::move(map_vec)));
  return map;
}

ObjectPtr ExpressionExecutor::ExecIdentifier(AstNode* node) {
  Identifier* id_node = static_cast<Identifier*>(node);
  const std::string& name = id_node->name();
  auto obj = symbol_table_stack().Lookup(name, false).Ref();
  return PassVar(obj, symbol_table_stack());
}

ObjectPtr ExpressionExecutor::ArrayAccess(Array& array_node,
                                          ArrayObject& obj) {
  // Executes index expression of array
  ObjectPtr index = Exec(array_node.index_exp());

  // Array accept only integer index
  if (index->type() != Object::ObjectType::INT) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("array index must be integer"));
  }

  // Gets the value of integer object
  int num = static_cast<IntObject*>(index.get())->value();

  auto val = static_cast<ArrayObject&>(obj).Element(size_t(num));
  return PassVar(val, symbol_table_stack());
}

ObjectPtr ExpressionExecutor::TupleAccess(Array& array_node,
                                          TupleObject& obj) {
  // Executes index expression of array
  ObjectPtr index = Exec(array_node.index_exp());

  // Array accept only integer index
  if (index->type() != Object::ObjectType::INT) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("tuple index must be integer"));
  }

  // Gets the value of integer object
  int num = static_cast<IntObject*>(index.get())->value();

  auto val = static_cast<TupleObject&>(obj).Element(size_t(num));
  return PassVar(val, symbol_table_stack());
}

ObjectPtr ExpressionExecutor::MapAccess(Array& array_node, MapObject& obj) {
  // Executes index expression of array
  ObjectPtr index = Exec(array_node.index_exp());

  auto val = static_cast<MapObject&>(obj).Element(index);
  return PassVar(val, symbol_table_stack());
}

ObjectPtr ExpressionExecutor::ExecArrayAccess(AstNode* node) {
  Array* array_node = static_cast<Array*>(node);
  Expression* arr_exp = array_node->arr_exp();

  ObjectPtr array_obj = Exec(arr_exp);

  if (array_obj->type() == Object::ObjectType::ARRAY) {
    return ArrayAccess(*array_node, static_cast<ArrayObject&>(*array_obj));
  } else if (array_obj->type() == Object::ObjectType::TUPLE) {
    return TupleAccess(*array_node, static_cast<TupleObject&>(*array_obj));
  } else if (array_obj->type() == Object::ObjectType::MAP) {
    return MapAccess(*array_node, static_cast<MapObject&>(*array_obj));
  } else {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("operator [] not overload for object"));
  }
}

ObjectPtr ExpressionExecutor::ExecFuncCall(FunctionCall* node) {
  FuncCallExecutor fcall_exec(this, symbol_table_stack());
  return fcall_exec.Exec(node);
}

ObjectPtr ExpressionExecutor::ExecLiteral(AstNode* node) {
  Literal* literal = static_cast<Literal*>(node);
  switch (literal->literal_type()) {
    case Literal::Type::kInteger: {
      ObjectPtr obj(obj_factory.NewInt(boost::get<int>(literal->value())));
      return obj;
    } break;

    case Literal::Type::kBool: {
      ObjectPtr obj(obj_factory.NewBool(boost::get<bool>(literal->value())));
      return obj;
    } break;

    case Literal::Type::kReal: {
      ObjectPtr obj(obj_factory.NewReal(boost::get<float>(literal->value())));
      return obj;
    } break;

    case Literal::Type::kString: {
    std::string str = boost::get<std::string>(literal->value());
      ObjectPtr obj(obj_factory.NewString(std::move(str)));
      return obj;
    } break;
  }
}

ObjectPtr ExpressionExecutor::ExecBinOp(BinaryOperation* node) {
  // Executes the left and right side of the expression
  ObjectPtr left = Exec(static_cast<AstNode*>(node->left()));
  ObjectPtr right = Exec(static_cast<AstNode*>(node->right()));

  ObjectPtr res;

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

    default: {
      throw RunTimeError(RunTimeError::ErrorCode::INVALID_OPCODE,
                         boost::format("invalid bin operation opcode"));
    }
  }

  return res;
}

ObjectPtr ExpressionExecutor::ExecAttribute(Attribute* node) {
  ObjectPtr exp = Exec(node->exp());
  std::string name = node->id()->name();

  return exp->Arrow(exp, name);
}

ObjectPtr ExpressionExecutor::ExecCmdExpr(CmdExpression* node) {
  CmdExecutor cmd_full(this, symbol_table_stack());
  std::tuple<int, std::string> res(cmd_full.ExecGetResult(
      static_cast<CmdFull*>(node->cmd())));

  // create command object
  int status = std::get<0>(res);
  std::string str = std::get<1>(res);
  ObjectFactory obj_factory(symbol_table_stack());
  ObjectPtr obj(obj_factory.NewCmdObj(status, std::move(str),
                                      std::move(std::string(""))));
  return obj;
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
    obj_vec.push_back(std::move(expr_executor.Exec(value)));
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
                         boost::format("object is not callable"));
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

}
}
