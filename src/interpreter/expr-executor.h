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

#ifndef SHPP_EXPR_EXECUTOR_H
#define SHPP_EXPR_EXECUTOR_H

#include <boost/filesystem.hpp>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "ast/ast.h"
#include "executor.h"
#include "objects/obj-type.h"
#include "objects/object-factory.h"

namespace shpp {
namespace internal {

class AssignableListExecutor : public Executor {
 public:
  AssignableListExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  std::vector<ObjectPtr> Exec(AstNode* node);

  ObjectPtr ExecAssignable(AstNode* node);

  void set_stop(StopFlag flag) override;
};

class ExpressionExecutor : public Executor {
 public:
  ExpressionExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack),
        pass_ref_(false),
        obj_factory_(symbol_table_stack) {}

  // Entry point to execute expression
  ObjectPtr Exec(AstNode* node, bool pass_ref = false);

  // Executes literal const and return an object with its value
  ObjectPtr ExecLiteral(AstNode* node);

  // Lookup on symbol table and return a reference
  // if the object is simple as integer, bool, string or real
  // then create a copy the object and return its reference
  // if it is a container as array, tuple, or map
  // return only the reference of the object on symbol table
  ObjectPtr ExecIdentifier(AstNode* node);

  ObjectPtr ExecVarEnvId(AstNode* node);

  // Executes array access, it could be a language array, map, tuple or
  // custon object
  ObjectPtr ExecArrayAccess(AstNode* node);

  // Executes array instantiation
  ObjectPtr ExecArrayInstantiation(AstNode* node);

  // Executes tuple instantiation
  ObjectPtr ExecTupleInstantiation(AstNode* node);

  // Executes map instantiation
  ObjectPtr ExecMapInstantiation(AstNode* node);

  // Executes function call
  ObjectPtr ExecFuncCall(FunctionCall* node);

  // Executes binary operation
  ObjectPtr ExecBinOp(BinaryOperation* node);

  // Executes attribute
  ObjectPtr ExecAttribute(Attribute* node);

  // Executes lambda
  ObjectPtr ExecLambdaFunc(AstNode* node);

  // Executes let expression
  ObjectPtr ExecLetExpression(LetExpression* node);

  // Executes cmd expression
  ObjectPtr ExecCmdExpr(CmdExpression* node);

  // Executes slice expression
  ObjectPtr ExecSlice(Slice* node);

  // Executes not expression, as the language has
  // to kinds of not expression, with not and !
  // the function has to check wich node is
  ObjectPtr ExecNotExpr(AstNode* node);

  ObjectPtr ExecUnary(AstNode* node);

  ObjectPtr ExecNull();

  ObjectPtr ExecGlob(Glob* glob);

  ObjectPtr ExecSpecialString(SpecialString* sstr);

  ObjectPtr ExecInstanceOf(ObjectPtr obj, ObjectPtr base);

  ObjectPtr ExecIs(ObjectPtr obj1, ObjectPtr obj2);

  ObjectPtr ExecListComprehension(AstNode* node);

  ObjectPtr ExecIfElseExpr(AstNode* node);

  void set_stop(StopFlag flag) override;

 private:
  bool pass_ref_;  // this attribute is only used to execute attribute expr
  ObjectFactory obj_factory_;
};

class GlobExecutor : public Executor {
 public:
  GlobExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  ObjectPtr Exec(Glob* glob_node);
};

class SpecialStringExecutor : public Executor {
 public:
  SpecialStringExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  ObjectPtr Exec(SpecialString* sstr_node);
};

class ExprListExecutor : public Executor {
 public:
  ExprListExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  std::vector<ObjectPtr> Exec(AstNode* node);

  void set_stop(StopFlag flag) override;
};

class EllipsisExprExecutor : public Executor {
 public:
  EllipsisExprExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  std::vector<ObjectPtr> Exec(AstNode* node);

  void set_stop(StopFlag flag) override;
};

class ArgumentsExecutor : public Executor {
 public:
  using Args = std::vector<ObjectPtr>;
  using KWArgs = std::unordered_map<std::string, ObjectPtr>;

  ArgumentsExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  // Entry point to execute arguments
  std::tuple<Args, KWArgs> Exec(ArgumentsList* args);

  void set_stop(StopFlag flag) override;

 protected:
  bool inside_loop() override { return false; }

  bool inside_switch() override { return false; }
};

class FuncCallExecutor : public Executor {
 public:
  using Args = std::vector<ObjectPtr>;
  using KWArgs = std::unordered_map<std::string, ObjectPtr>;

  FuncCallExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  // Entry point to execute expression
  ObjectPtr Exec(FunctionCall* node);

  void set_stop(StopFlag flag) override;

 protected:
  bool inside_loop() override { return false; }

  bool inside_switch() override { return false; }
};

class ListComprehensionExecutor : public Executor {
 public:
  ListComprehensionExecutor(Executor* parent,
                            SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  ObjectPtr Exec(AstNode* node);

  std::unique_ptr<Statement> MountBlock(ListComprehension* list_comp_node);

  std::unique_ptr<Statement> MountIfBlock(CompIf* comp_if,
                                          ListComprehension* list_comp_node);

  std::unique_ptr<Statement> ExecForIfList(
      std::vector<Expression*>& for_if_list,
      std::unique_ptr<Statement>&& stmt_l, AstNodeFactory& ast_node_factory);

  std::unique_ptr<Statement> MountForBlock(CompFor* comp_for,
                                           ListComprehension* list_comp_node);
};

}  // namespace internal
}  // namespace shpp

#endif  // SHPP_EXPR_EXECUTOR_H
