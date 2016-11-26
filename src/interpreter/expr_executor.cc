#include "expr_executor.h"

#include <string>
#include <boost/variant.hpp>

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

void ExpressionExecutor::set_stop(StopFlag flag) {
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
  if (flag == StopFlag::kThrow) {
    parent()->set_stop(flag);
  }
}

}
}
