#include "assign-executor.h"

#include <string>
#include <boost/variant.hpp>

#include "expr-executor.h"
#include "objects/func-object.h"

namespace setti {
namespace internal {

void AssignExecutor::Exec(AstNode* node) {
  AssignmentStatement* assign_node = static_cast<AssignmentStatement*>(node);

  if (!assign_node->has_rvalue()) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("not valid left side expression"));
  }

  TokenKind assign_kind = assign_node->assign_kind();

  // Executes the left side of assignment
  auto vars = AssignList(assign_node->lexp_list());

  // Executes the right sid of assignment
  AssignableListExecutor assignables(this, symbol_table_stack());
  auto values = assignables.Exec(assign_node->rvalue_list());

  // Assignment can be done only when the tuples have the same size
  // or there is only one variable on the left side
  // a, b, c = 1, 2, 3; a = 1, 2, 3; a, b = f
  if ((vars.size() != 1) && (values.size() != 1) &&
      (vars.size() != values.size())) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("different size of tuples"));
  }

  if ((vars.size() == 1) && (values.size() == 1)) {
    AssignOperation(vars[0], values[0], assign_kind);
  } else if ((vars.size() == 1) && (values.size() != 1)) {
    ObjectPtr tuple_obj(obj_factory_.NewTuple(std::move(values)));
    AssignOperation(vars[0], tuple_obj, assign_kind);
  } else if ((vars.size() != 1) && (values.size() == 1)) {
    // only tuple object is accept on this case
    if (values[0]->type() != Object::ObjectType::TUPLE) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("expect tuple object as rvalue"));
    }

    // numbers of variables must be equal the number of tuple elements
    TupleObject &tuple_obj = static_cast<TupleObject&>(*values[0]);
    if (vars.size() != tuple_obj.Size()) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
          boost::format("numbers of variables: %1% and "
                        "numbers of tuples: %2% are "
                        "incompatibles")% vars.size() % tuple_obj.Size());
    }

    for (size_t i = 0; i < vars.size(); i++) {
      AssignOperation(vars[i], tuple_obj.Element(i), assign_kind);
    }
  } else {
    // on this case there are the same number of variables and values
    for (size_t i = 0; i < vars.size(); i++) {
      AssignOperation(vars[i], values[i], assign_kind);
    }
  }
}

void AssignExecutor::AssignOperation(std::reference_wrapper<ObjectPtr> ref,
                                     ObjectPtr value, TokenKind token) {
  switch (token) {
    case TokenKind::ASSIGN:
      ref.get() = value;
      break;

    case TokenKind::ASSIGN_BIT_OR:
      ref.get() = ref.get()->BitOr(value);
      break;

    case TokenKind::ASSIGN_BIT_XOR:
      ref.get() = ref.get()->BitXor(value);
      break;

    case TokenKind::ASSIGN_BIT_AND:
      ref.get() = ref.get()->BitAnd(value);
      break;

    case TokenKind::ASSIGN_SHL:
      ref.get() = ref.get()->LeftShift(value);
      break;

    case TokenKind::ASSIGN_SAR:
      ref.get() = ref.get()->RightShift(value);
      break;

    case TokenKind::ASSIGN_ADD:
      ref.get() = ref.get()->Add(value);
      break;

    case TokenKind::ASSIGN_SUB:
      ref.get() = ref.get()->Sub(value);
      break;

    case TokenKind::ASSIGN_MUL:
      ref.get() = ref.get()->Mult(value);
      break;

    case TokenKind::ASSIGN_DIV:
      ref.get() = ref.get()->Div(value);
      break;

    case TokenKind::ASSIGN_MOD:
      ref.get() = ref.get()->DivMod(value);
      break;

    default:
      throw RunTimeError(RunTimeError::ErrorCode::INVALID_OPCODE,
                         boost::format("not valid assignment operation"));
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

ObjectPtr& AssignExecutor::RefMap(Array& array_node, MapObject& obj) {
  // Executes index expression of map
  ExpressionExecutor expr_exec(this, symbol_table_stack());
  ObjectPtr index(expr_exec.Exec(array_node.index_exp()));

  return static_cast<MapObject&>(obj).ElementRef(index);
}

ObjectPtr& AssignExecutor::AssignAtrribute(AstNode* node) {
  Attribute* att_node = static_cast<Attribute*>(node);
  Expression* att_exp = att_node->exp();

  ExpressionExecutor expr_exec(this, symbol_table_stack());
  ObjectPtr exp_obj(expr_exec.Exec(att_exp));

  return exp_obj->AttrAssign(exp_obj, att_node->id()->name());
}

// TODO: Executes for map and custon objects
ObjectPtr& AssignExecutor::AssignArray(AstNode* node) {
  Array* array_node = static_cast<Array*>(node);
  Expression* arr_exp = array_node->arr_exp();

  ObjectPtr& obj = AssignmentAcceptorExpr(arr_exp);

  if (obj->type() == Object::ObjectType::ARRAY) {
    return RefArray(*array_node, *static_cast<ArrayObject*>(obj.get()));
  } else if (obj->type() == Object::ObjectType::TUPLE) {
    return RefTuple(*array_node, *static_cast<TupleObject*>(obj.get()));
  } else if (obj->type() == Object::ObjectType::MAP) {
    return RefMap(*array_node, *static_cast<MapObject*>(obj.get()));
  }

  throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                     boost::format("__get__element not overload"));
}

ObjectPtr& AssignExecutor::AssignmentAcceptorExpr(AstNode* node) {
  switch(node->type()) {
    case AstNode::NodeType::kIdentifier:
      return AssignIdentifier(node, true);
      break;

    case AstNode::NodeType::kArray:
      return AssignArray(node);
      break;

    case AstNode::NodeType::kAttribute:
      return AssignAtrribute(node);
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
        std::reference_wrapper<ObjectPtr>(AssignmentAcceptorExpr(exp)));
  }

  return vec;
}

void AssignExecutor::set_stop(StopFlag flag) {
  parent()->set_stop(flag);
}

}
}
