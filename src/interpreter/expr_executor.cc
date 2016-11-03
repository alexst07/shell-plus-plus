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

ObjectPtr ExpressionExecutor::Exec(AstNode* node) {
  switch (node->type()) {
    case AstNode::NodeType::kLiteral: {
      return ExecLiteral(node);
    } break;

    case AstNode::NodeType::kIdentifier: {
      return ExecIdentifier(node);
    } break;

    case AstNode::NodeType::kArrayInstantiation: {
      return ExecArrayInstantiation(node);
    } break;
  }
}

ObjectPtr ExpressionExecutor::ExecArrayInstantiation(AstNode* node) {
  ArrayInstantiation* array_node = static_cast<ArrayInstantiation*>(node);
  AssignableListExecutor assignable_list(this, symbol_table_stack());
  auto vec = assignable_list.Exec(array_node->assignable_list());
  std::unique_ptr<Object> array_obj(new ArrayObject(std::move(vec)));
  return array_obj;
}

ObjectPtr ExpressionExecutor::ExecIdentifier(AstNode* node) {
  Identifier* id_node = static_cast<Identifier*>(node);
  const std::string& name = id_node->name();
  auto obj = symbol_table_stack().Lookup(name, false).Ref();
  return PassVar(obj);
}

ObjectPtr ExpressionExecutor::ExecLiteral(AstNode* node) {
  Literal* literal = static_cast<Literal*>(node);
  switch (literal->literal_type()) {
    case Literal::Type::kInteger: {
      ObjectPtr obj(new IntObject(boost::get<int>(literal->value())));
      return obj;
    } break;

    case Literal::Type::kBool: {
      ObjectPtr obj(new BoolObject(boost::get<bool>(literal->value())));
      return obj;
    } break;

    case Literal::Type::kReal: {
      ObjectPtr obj(new RealObject(boost::get<float>(literal->value())));
      return obj;
    } break;

    case Literal::Type::kString: {
    std::string str = boost::get<std::string>(literal->value());
      ObjectPtr obj(new StringObject(std::move(str)));
      return obj;
    } break;
  }
}


}
}
