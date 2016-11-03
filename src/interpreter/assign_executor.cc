#include "assign_executor.h"

#include <string>
#include <boost/variant.hpp>

#include "expr_executor.h"
#include "ast/symbol_table.h"

namespace setti {
namespace internal {

void AssignExecutor::Exec(AstNode* node) {
  AssignmentStatement* assign_node = static_cast<AssignmentStatement*>(node);

  if (!assign_node->has_rvalue()) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("not valid left side expression"));
  }

  // Executes the left side of assignment
  auto vars = AssignList(assign_node->lexp_list());

  // Executes the right sid of assignment
  AssignableListExecutor assignables(this, symbol_table_stack());
  auto values = assignables.Exec(assign_node->rvalue_list());

  // Assignment can be done only when the tuples have the same size
  // or there is only one variable on the left side
  // a, b, c = 1, 2, 3 or a = 1, 2, 3
  if ((vars.size() != 1) && (vars.size() != values.size())) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("different size of tuples"));
  }

  if ((vars.size() == 1) && (values.size() == 1)) {
    vars[0].get() = values[0];
  } else if ((vars.size() == 1) && (values.size() != 1)) {
    ObjectPtr tuple_obj(new TupleObject(std::move(values)));
    vars[0].get() = tuple_obj;
  } else {
    // on this case there are the same number of variables and values
    for (size_t i = 0; i < vars.size(); i++) {
      vars[i].get() = values[i];
    }
  }
}

ObjectPtr& AssignExecutor::AssignIdentifier(AstNode* node, bool create) {
  Identifier* id_node = static_cast<Identifier*>(node);
  const std::string& name = id_node->name();
  return symbol_table_stack().Lookup(name, create).Ref();
}

ObjectPtr& AssignExecutor::RefArray(Array& array_node, ArrayObject& obj) {
  // Executes index expression of array
  ExpressionExecutor expr_exec(this, symbol_table_stack());
  ObjectPtr index(expr_exec.Exec(array_node.index_exp()));

  // Array accept only integer index
  if (index->type() != Object::ObjectType::INT) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("array index must be integer"));
  }

  // Gets the value of integer object
  int num = static_cast<IntObject*>(index.get())->value();

  return static_cast<ArrayObject&>(obj).ElementRef(size_t(num));
}

ObjectPtr& AssignExecutor::RefTuple(Array& array_node, TupleObject& obj) {
  // Executes index expression of tuple
  ExpressionExecutor expr_exec(this, symbol_table_stack());
  ObjectPtr index(expr_exec.Exec(array_node.index_exp()));

  // Tuple accept only integer index
  if (index->type() != Object::ObjectType::INT) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("tuple index must be integer"));
  }

  // Gets the value of integer object
  int num = static_cast<IntObject*>(index.get())->value();

  return static_cast<TupleObject&>(obj).ElementRef(size_t(num));
}

// TODO: Executes for map and custon objects
ObjectPtr& AssignExecutor::AssignArray(AstNode* node) {
  Array* array_node = static_cast<Array*>(node);
  Expression* arr_exp = array_node->arr_exp();

  // Interprete case as a[1] = ?
  // where array expression is an identifier
  if (arr_exp->type() == AstNode::NodeType::kIdentifier) {
    ObjectPtr& obj = AssignIdentifier(arr_exp);

    if (obj->type() == Object::ObjectType::ARRAY) {
      return RefArray(*array_node, *static_cast<ArrayObject*>(obj.get()));
    } else if (obj->type() == Object::ObjectType::TUPLE) {
      return RefTuple(*array_node, *static_cast<TupleObject*>(obj.get()));
    }
  } else if (arr_exp->type() == AstNode::NodeType::kArray) {
    // Interprete case as a[1]...[1] = ?
    // where array expression is an array
    ObjectPtr obj = AssignArray(arr_exp);

    if (obj->type() == Object::ObjectType::ARRAY) {
      return RefArray(*array_node, static_cast<ArrayObject&>(*obj));
    } else if (obj->type() == Object::ObjectType::TUPLE) {
      return RefTuple(*array_node, static_cast<TupleObject&>(*obj));
    }
  }
}

ObjectPtr& AssignExecutor::LeftVar(AstNode* node) {
  switch(node->type()) {
    case AstNode::NodeType::kIdentifier:
      return AssignIdentifier(node, true);
    break;

    case AstNode::NodeType::kArray:
      return AssignArray(node);
    break;

    default:
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("not valid left side expression"));
  }
}

std::vector<std::reference_wrapper<ObjectPtr>>
AssignExecutor::AssignList(AstNode* node) {
  ExpressionList* node_list = static_cast<ExpressionList*>(node);
  std::vector<std::reference_wrapper<ObjectPtr>> vec;

  for (Expression* exp: node_list->children()) {
    vec.push_back(
        std::reference_wrapper<ObjectPtr>(LeftVar(exp)));
  }

  return vec;
}

}
}
