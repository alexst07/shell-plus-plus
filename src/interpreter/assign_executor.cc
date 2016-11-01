#include "assign_executor.h"

#include <string>
#include <boost/variant.hpp>

#include "expr_executor.h"
#include "ast/symbol_table.h"

namespace setti {
namespace internal {

void AssignExecutor::Exec(AstNode* node) {
  AssignmentStatement* assign_node = static_cast<AssignmentStatement*>(node);
}

SymbolAttr& AssignExecutor::AssignIdentifier(AstNode* node) {
  Identifier* id_node = static_cast<Identifier*>(node);

  const std::string& name = id_node->name();
  return symbol_table_stack().Lookup(name);
}

std::unique_ptr<Object>& AssignExecutor::ObjectArray(Array& array_node,
                                                     ArrayObject& obj) {
  // Executes index expression of array
  ExpressionExecutor expr_exec(this, symbol_table_stack());
  std::unique_ptr<Object> index(expr_exec.Exec(array_node.index_exp()));

  // Array accept only integer index
  if (index->type() != Object::ObjectType::INT) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("array index must be integer"));
  }

  // Gets the value of integer object
  int num = static_cast<IntObject*>(index.get())->value();

  return static_cast<ArrayObject&>(obj).ElementRef(size_t(num));
}

// TODO: Executes for map and custon objects
std::unique_ptr<Object>& AssignExecutor::AssignArray(AstNode* node) {
  Array* array_node = static_cast<Array*>(node);
  Expression* arr_exp = array_node->arr_exp();

  // Interprete case as a[1] = ?
  // where array expression is an identifier
  if (arr_exp->type() == AstNode::NodeType::kIdentifier) {
    SymbolAttr& symbol = AssignIdentifier(arr_exp);
    ArrayObject* obj = static_cast<ArrayObject*>(symbol.value());

    if (obj->type() == Object::ObjectType::ARRAY) {
      return ObjectArray(*array_node, *obj);
    }
  } else if (arr_exp->type() == AstNode::NodeType::kArray) {
    // Interprete case as a[1]...[1] = ?
    // where array expression is an array
    std::unique_ptr<Object>& obj = AssignArray(arr_exp);

    if (obj->type() == Object::ObjectType::ARRAY) {
      return ObjectArray(*array_node, static_cast<ArrayObject&>(*obj));
    }
  }
}

std::unique_ptr<Object>& AssignExecutor::LeftVar(AstNode* node) {
  switch(node->type()) {
    case AstNode::NodeType::kIdentifier:
      return AssignIdentifier(node).RefValue();
    break;

    case AstNode::NodeType::kArray:
      return AssignArray(node);
    break;

    default:
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("not valid left side expression"));
  }
}

std::vector<std::reference_wrapper<std::unique_ptr<Object>>>
AssignExecutor::AssignList(AstNode* node) {
  ExpressionList* node_list = static_cast<ExpressionList*>(node);
  std::vector<std::reference_wrapper<std::unique_ptr<Object>>> vec;

  for (Expression* exp: node_list->children()) {
    vec.push_back(
        std::reference_wrapper<std::unique_ptr<Object>>(LeftVar(exp)));
  }

  return vec;
}

std::vector<std::unique_ptr<Object>> AssignExecutor::ExecAssignableList(
    AstNode* node) {
  AssignableList* assign_list_node = static_cast<AssignableList*>(node);

  std::vector<std::unique_ptr<Object>> obj_vec;

  for (AstNode* value: assign_list_node->children()) {
    obj_vec.push_back(std::move(ExecAssignable(value)));
  }

  return obj_vec;
}

std::unique_ptr<Object> AssignExecutor::ExecAssignable(AstNode* node) {
  if (AstNode::IsExpression(node->type())) {
    ExpressionExecutor expr_exec(this, symbol_table_stack());
    return expr_exec.Exec(node);
  }
}

}
}
