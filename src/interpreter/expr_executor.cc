#include "expr_executor.h"

#include <string>
#include <boost/variant.hpp>

namespace setti {
namespace internal {

std::vector<std::unique_ptr<Object>> AssignableListExecutor::Exec(
    AstNode* node) {
  AssignableList* assign_list_node = static_cast<AssignableList*>(node);

  std::vector<std::unique_ptr<Object>> obj_vec;

  for (AstNode* value: assign_list_node->children()) {
    obj_vec.push_back(std::move(ExecAssignable(value)));
  }

  return obj_vec;
}

std::unique_ptr<Object> AssignableListExecutor::ExecAssignable(AstNode* node) {
  AssignableValue* assignable_node = static_cast<AssignableValue*>(node);
  if (AstNode::IsExpression(node->type())) {
    ExpressionExecutor expr_exec(this, symbol_table_stack());
    return expr_exec.Exec(assignable_node->value());
  }
}

std::unique_ptr<Object> ExpressionExecutor::Exec(AstNode* node) {
  switch (node->type()) {
    case AstNode::NodeType::kLiteral: {
      return ExecLiteral(node);
    } break;
  }
}

std::unique_ptr<Object> ExpressionExecutor::ExecLiteral(AstNode* node) {
  Literal* literal = static_cast<Literal*>(node);
  switch (literal->literal_type()) {
    case Literal::Type::kInteger: {
      std::unique_ptr<Object> obj(
          new IntObject(boost::get<int>(literal->value())));
      return obj;
    } break;

    case Literal::Type::kBool: {
      std::unique_ptr<Object> obj(
          new BoolObject(boost::get<bool>(literal->value())));
      return obj;
    } break;

    case Literal::Type::kReal: {
      std::unique_ptr<Object> obj(
          new RealObject(boost::get<float>(literal->value())));
      return obj;
    } break;

    case Literal::Type::kString: {
    std::string str = boost::get<std::string>(literal->value());
      std::unique_ptr<Object> obj(
          new StringObject(std::move(str)));
      return obj;
    } break;
  }
}


}
}
