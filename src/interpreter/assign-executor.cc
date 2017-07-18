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

#include "assign-executor.h"

#include <string>
#include <boost/variant.hpp>

#include "expr-executor.h"
#include "objects/func-object.h"

namespace shpp {
namespace internal {

// TODO: when c++17 be avaiable this method can be joined with Exec using
// if constexpr
ObjectPtr AssignExecutor::ExecWithReturn(AstNode* node) {
  AssignmentStatement* assign_node = static_cast<AssignmentStatement*>(node);

  if (!assign_node->has_rvalue()) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("not valid right side expression"));
  }

  TokenKind assign_kind = assign_node->assign_kind();

  // Executes the right sid of assignment
  AssignableListExecutor assignables(this, symbol_table_stack());
  auto values = assignables.Exec(assign_node->rvalue_list());

  ExpressionList* left_exp_list = assign_node->lexp_list();
  size_t num_left_exp = left_exp_list->num_children();
  std::vector<Expression*> left_exp_vec = left_exp_list->children();

  Assign(left_exp_vec, values, assign_kind);

  if (values.size() == 1) {
    // if there are only one expression on right side, so retun this object
    return values[0];
  } else if (values.size() > 1) {
    // if there are more than one expression on right side, put all terms
    // on a tuple, and return this tuple
    return obj_factory_.NewTuple(std::move(values));
  }

  throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                     boost::format("not valid right side expression"));
}

void AssignExecutor::Exec(AstNode* node) {
  AssignmentStatement* assign_node = static_cast<AssignmentStatement*>(node);

  if (!assign_node->has_rvalue()) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("not valid right side expression"));
  }

  TokenKind assign_kind = assign_node->assign_kind();

  // Executes the right sid of assignment
  AssignableListExecutor assignables(this, symbol_table_stack());
  auto values = assignables.Exec(assign_node->rvalue_list());

  ExpressionList* left_exp_list = assign_node->lexp_list();
  size_t num_left_exp = left_exp_list->num_children();
  std::vector<Expression*> left_exp_vec = left_exp_list->children();

  Assign(left_exp_vec, values, assign_kind);
}

void AssignExecutor::Assign(std::vector<Expression*>& left_exp_vec,
                            std::vector<ObjectPtr>& values,
                            TokenKind assign_kind) {
  size_t num_left_exp = left_exp_vec.size();

  // Assignment can be done only when the tuples have the same size
  // or there is only one variable on the left side
  // a, b, c = 1, 2, 3; a = 1, 2, 3; a, b = f
  if ((num_left_exp != 1) && (values.size() != 1) &&
      (num_left_exp != values.size())) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("different size of tuples"));
  }

  if ((num_left_exp == 1) && (values.size() == 1)) {
    AssignOperation(left_exp_vec[0], values[0], assign_kind);
  } else if ((num_left_exp == 1) && (values.size() != 1)) {
    ObjectPtr tuple_obj(obj_factory_.NewTuple(std::move(values)));
    AssignOperation(left_exp_vec[0], tuple_obj, assign_kind);
  } else if ((num_left_exp != 1) && (values.size() == 1)) {
    // only tuple object is accept on this case
    if (values[0]->type() != Object::ObjectType::TUPLE) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("expect tuple object as rvalue"));
    }

    // numbers of variables must be equal the number of tuple elements
    TupleObject &tuple_obj = static_cast<TupleObject&>(*values[0]);
    if (num_left_exp != tuple_obj.Size()) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
          boost::format("numbers of variables: %1% and "
                        "numbers of tuples: %2% are "
                        "incompatibles")% num_left_exp % tuple_obj.Size());
    }

    for (size_t i = 0; i < num_left_exp; i++) {
      AssignOperation(left_exp_vec[i], tuple_obj.Element(i), assign_kind);
    }
  } else {
    // on this case there are the same number of variables and values
    for (size_t i = 0; i < num_left_exp; i++) {
      AssignOperation(left_exp_vec[i], values[i], assign_kind);
    }
  }
}

void AssignExecutor::AssignOperation(Expression* left_exp, ObjectPtr value,
                                     TokenKind token) {
  AssignmentAcceptorExpr(left_exp, value, token);
}

void AssignExecutor::AssignIdentifier(AstNode* node, ObjectPtr value,
                                      TokenKind token, bool create) {
  Identifier* id_node = static_cast<Identifier*>(node);
  const std::string& name = id_node->name();
  ObjectPtr& ref = symbol_table_stack().Lookup(name, create).Ref();
  AssignToRef(ref, value, token);
}

void AssignExecutor::AssignAtrribute(AstNode* node, ObjectPtr value,
                                     TokenKind token) {
  Attribute* att_node = static_cast<Attribute*>(node);
  Expression* att_exp = att_node->exp();

  ExpressionExecutor expr_exec(this, symbol_table_stack());
  ObjectPtr exp_obj(expr_exec.Exec(att_exp));

  ObjectPtr& ref = exp_obj->AttrAssign(exp_obj, att_node->id()->name());
  AssignToRef(ref, value, token);
}

void AssignExecutor::AssignArray(AstNode* node, ObjectPtr value,
                                 TokenKind token) {
  Array* array_node = static_cast<Array*>(node);
  Expression* arr_exp = array_node->arr_exp();

  ExpressionExecutor expr_exec(this, symbol_table_stack());
  ObjectPtr arr_obj = expr_exec.Exec(arr_exp, true);
  ObjectPtr index = expr_exec.Exec(array_node->index_exp(), true);

  AssignToArray(arr_obj, index, value, token);
}

void AssignExecutor::AssignmentAcceptorExpr(AstNode* node, ObjectPtr value,
                                            TokenKind token) {
  switch(node->type()) {
    case AstNode::NodeType::kIdentifier:
      AssignIdentifier(node, value, token, true);
      break;

    case AstNode::NodeType::kArray:
      AssignArray(node, value, token);
      break;

    case AstNode::NodeType::kAttribute:
      AssignAtrribute(node, value, token);
      break;

    default:
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("not valid left side expression"));
  }
}

void AssignExecutor::set_stop(StopFlag flag) {
  parent()->set_stop(flag);
}

void AssignExecutor::AssignToRef(ObjectPtr& ref, ObjectPtr value,
                                 TokenKind token) {
  switch (token) {
    case TokenKind::ASSIGN:
      ref = value;
      break;

    case TokenKind::ASSIGN_BIT_OR:
      ref = ref->BitOr(value);
      break;

    case TokenKind::ASSIGN_BIT_XOR:
      ref = ref->BitXor(value);
      break;

    case TokenKind::ASSIGN_BIT_AND:
      ref = ref->BitAnd(value);
      break;

    case TokenKind::ASSIGN_SHL:
      ref = ref->LeftShift(value);
      break;

    case TokenKind::ASSIGN_SAR:
      ref = ref->RightShift(value);
      break;

    case TokenKind::ASSIGN_ADD:
      ref = ref->Add(value);
      break;

    case TokenKind::ASSIGN_SUB:
      ref = ref->Sub(value);
      break;

    case TokenKind::ASSIGN_MUL:
      ref = ref->Mult(value);
      break;

    case TokenKind::ASSIGN_DIV:
      ref = ref->Div(value);
      break;

    case TokenKind::ASSIGN_MOD:
      ref = ref->DivMod(value);
      break;

    default:
      throw RunTimeError(RunTimeError::ErrorCode::INVALID_OPCODE,
                         boost::format("not valid assignment operation"));
  }
}

void AssignExecutor::AssignToArray(ObjectPtr arr, ObjectPtr index,
                                   ObjectPtr value, TokenKind token) {
  switch (token) {
    case TokenKind::ASSIGN:
      arr->SetItem(index, value);
      break;

    case TokenKind::ASSIGN_BIT_OR:
      arr->SetItem(index, arr->GetItem(index)->BitOr(value));
      break;

    case TokenKind::ASSIGN_BIT_XOR:
      arr->SetItem(index, arr->GetItem(index)->BitXor(value));
      break;

    case TokenKind::ASSIGN_BIT_AND:
      arr->SetItem(index, arr->GetItem(index)->BitAnd(value));
      break;

    case TokenKind::ASSIGN_SHL:
      arr->SetItem(index, arr->GetItem(index)->LeftShift(value));
      break;

    case TokenKind::ASSIGN_SAR:
      arr->SetItem(index, arr->GetItem(index)->RightShift(value));
      break;

    case TokenKind::ASSIGN_ADD:
      arr->SetItem(index, arr->GetItem(index)->Add(value));
      break;

    case TokenKind::ASSIGN_SUB:
      arr->SetItem(index, arr->GetItem(index)->Sub(value));
      break;

    case TokenKind::ASSIGN_MUL:
      arr->SetItem(index, arr->GetItem(index)->Mult(value));
      break;

    case TokenKind::ASSIGN_DIV:
      arr->SetItem(index, arr->GetItem(index)->Div(value));
      break;

    case TokenKind::ASSIGN_MOD:
      arr->SetItem(index, arr->GetItem(index)->DivMod(value));
      break;

    default:
      throw RunTimeError(RunTimeError::ErrorCode::INVALID_OPCODE,
                         boost::format("not valid assignment operation"));
  }
}

}
}
