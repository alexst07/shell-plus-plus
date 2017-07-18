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

#ifndef SHPP_ASSIGN_EXECUTOR_H
#define SHPP_ASSIGN_EXECUTOR_H

#include <string>
#include <memory>
#include <vector>
#include <tuple>

#include "ast/ast.h"
#include "objects/obj-type.h"
#include "executor.h"
#include "symbol-table.h"
#include "objects/object-factory.h"

namespace shpp {
namespace internal {

// Class to execute assignment operation
//
// on this class some methods return reference for shared_ptr
// it is not common in C++, but on this case, the objective
// is not increment the counter, but change the variable
class AssignExecutor: public Executor {
 public:
  AssignExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack)
      , obj_factory_(symbol_table_stack) {}

  // Entry point to execute assign operations
  void Exec(AstNode* node);

  // Execute assign operation as exec and return the the right side as object
  ObjectPtr ExecWithReturn(AstNode* node);

  void Assign(std::vector<Expression*>& left_exp_vec,
              std::vector<ObjectPtr>& values,
              TokenKind assign_kind = TokenKind::ASSIGN);

  void AssignIdentifier(AstNode* node, ObjectPtr value, TokenKind token,
                        bool create = false);

  void AssignOperation(Expression* left_exp, ObjectPtr value, TokenKind token);

  void AssignAtrribute(AstNode* node, ObjectPtr value, TokenKind token);

  void AssignArray(AstNode* node, ObjectPtr value, TokenKind token);

  void AssignmentAcceptorExpr(AstNode* node, ObjectPtr value, TokenKind token);

  void set_stop(StopFlag flag);

  void AssignToRef(ObjectPtr& ref, ObjectPtr value, TokenKind token);

  void AssignToArray(ObjectPtr arr, ObjectPtr index, ObjectPtr value,
                     TokenKind token);

 private:
  ObjectFactory obj_factory_;
};

}
}

#endif  // SHPP_ASSIGN_EXECUTOR_H
