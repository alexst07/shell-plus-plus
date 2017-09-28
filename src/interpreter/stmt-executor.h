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

#ifndef SHPP_STMT_EXECUTOR_H
#define SHPP_STMT_EXECUTOR_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <tuple>

#include "ast/ast.h"
#include "objects/obj-type.h"
#include "executor.h"
#include "objects/object-factory.h"

namespace shpp {
namespace internal {

class StmtListExecutor: public Executor {
 public:
  StmtListExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack)
      , stop_flag_(StopFlag::kGo)
      , exec_from_root_scope_(false) {}

  StmtListExecutor(bool exec_from_root_scope, Executor* parent,
      SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack)
      , stop_flag_(StopFlag::kGo)
      , exec_from_root_scope_(exec_from_root_scope) {}

  void Exec(AstNode* node);

  void set_stop(StopFlag flag) override;

  bool inside_root_scope() override {
    return exec_from_root_scope_;
  }

 private:
  StopFlag stop_flag_;
  bool exec_from_root_scope_;
};

class FuncDeclExecutor: public Executor {
 public:
  FuncDeclExecutor(Executor* parent, SymbolTableStack& symbol_table_stack,
                   bool method = false, bool lambda = false,
                   bool fstatic = false)
      : Executor(parent, symbol_table_stack)
      , obj_factory_(symbol_table_stack)
      , method_(method)
      , lambda_(lambda)
      , fstatic_(fstatic) {}

  void Exec(AstNode* node);

  ObjectPtr FuncObj(AstNode* node);

  template<class T>
  ObjectPtr FuncObjAux(T node);

  void set_stop(StopFlag flag) override;

 protected:
  bool inside_loop() override {
    return false;
  }

  bool inside_switch() override {
    return false;
  }

  bool inside_func() override {
    // lambda functions can change variables, even if the variables
    // is not global
    if (lambda_) {
      return false;
    }

    return true;
  }

 private:
  ObjectFactory obj_factory_;
  bool method_;
  bool lambda_;
  bool fstatic_;
};

class ClassDeclExecutor: public Executor {
 public:
  ClassDeclExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack)
      , obj_factory_(symbol_table_stack) {}

  void Exec(AstNode* node, bool inner = false,
      ObjectPtr inner_type_obj = ObjectPtr(nullptr));

  void ExecVarDecl(AstNode* node, DeclClassType& decl_class);

  ObjectPtr SuperClass(Expression* super);

  void set_stop(StopFlag flag) override;

 private:
  ObjectFactory obj_factory_;
};

class InterfaceDeclExecutor: public Executor {
 public:
  InterfaceDeclExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack)
      , obj_factory_(symbol_table_stack) {}

  void Exec(AstNode* node);

  static std::vector<ObjectPtr> HandleInterfaces(Executor* parent,
      ExpressionList* ifaces_node, SymbolTableStack& symbol_table_stack);

 private:
  ObjectFactory obj_factory_;
};

class StmtExecutor: public Executor {
 public:
  StmtExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  // Entry point to execute expression
  void Exec(AstNode* node);

  void set_stop(StopFlag flag) override;

  bool inside_root_scope() override {
    if (parent() != nullptr) {
      return parent()->inside_root_scope();
    }

    return false;
  }
};

class ReturnExecutor: public Executor {
 public:
  ReturnExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack)
      , obj_factory_(symbol_table_stack) {}

  // Entry point to execute expression
  void Exec(AstNode* node);

  void set_stop(StopFlag flag) override;

 private:
  ObjectFactory obj_factory_;
};

class TryCatchExecutor: public Executor {
 public:
  TryCatchExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  // Entry point to execute expression
  void Exec(TryCatchStatement* node);

  bool IsInstanceOfCaseObject(std::vector<ObjectPtr>& obj_res_list,
      ObjectPtr& ojb_excpt);

  void InsertCatchVar(const std::string& name, ObjectPtr& ojb_excpt,
      Position pos);

  void set_stop(StopFlag flag) override;
};

class ThrowExecutor: public Executor {
 public:
  ThrowExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  // Entry point to execute expression
  void Exec(ThrowStatement* node);
};

class IfElseExecutor: public Executor {
 public:
  IfElseExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  // Entry point to execute expression
  void Exec(IfStatement* node);

  void set_stop(StopFlag flag) override;
};

class WhileExecutor: public Executor {
 public:
  WhileExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack)
      , stop_flag_(StopFlag::kGo) {}

  // Entry point to execute while
  void Exec(WhileStatement* node);

  void set_stop(StopFlag flag) override;

 protected:
  bool inside_loop() override {
    return true;
  }

  bool inside_switch() override {
    return false;
  }

 private:
  StopFlag stop_flag_;
};

class ForInExecutor: public Executor {
 public:
  ForInExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack)
      , stop_flag_(StopFlag::kGo){}

  // Entry point to execute for in
  void Exec(ForInStatement *node);

  void Assign(std::vector<Expression*>& exp_list,
      std::vector<ObjectPtr>& it_values);

  void set_stop(StopFlag flag) override;

 protected:
  bool inside_loop() override {
    return true;
  }

  bool inside_switch() override {
    return false;
  }

 private:
  StopFlag stop_flag_;
};

class BreakExecutor: public Executor {
 public:
  BreakExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  void Exec(BreakStatement *node);

  void set_stop(StopFlag flag) override;
};

class ContinueExecutor: public Executor {
 public:
  ContinueExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  void Exec(ContinueStatement *node);

  void set_stop(StopFlag flag) override;
};

class SwitchExecutor: public Executor {
 public:
  SwitchExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  // Entry point to execute while
  void Exec(SwitchStatement* node);

  void set_stop(StopFlag flag) override;

 protected:
  bool inside_loop() override {
    return false;
  }

  bool inside_switch() override {
    return true;
  }

 private:
  bool MatchAnyExp(ObjectPtr exp, std::vector<ObjectPtr> &&exp_list);
};

class DeferExecutor: public Executor {
 public:
  DeferExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  void Exec(DeferStatement *node);

  void set_stop(StopFlag flag) override;
};

class CmdDeclExecutor: public Executor {
 public:
  CmdDeclExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack)
      , obj_factory_(symbol_table_stack) {}

  void Exec(AstNode* node);

  void set_stop(StopFlag flag) override;

 protected:
  bool inside_loop() override {
    return false;
  }

  bool inside_switch() override {
    return false;
  }

 private:
  ObjectFactory obj_factory_;
};

class ImportExecutor: public Executor {
 public:
  ImportExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  void Exec(ImportStatement *node);

  ObjectPtr ProcessModule(const std::string& module, const std::string& path);

  void set_stop(StopFlag flag) override;
};

class AliasDeclExecutor: public Executor {
 public:
  AliasDeclExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  void Exec(AliasDeclaration *node);
};

class DelStmtExecutor: public Executor {
 public:
  DelStmtExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  void Exec(DelStatement *node);

  void Del(Expression* node);

  void DelId(Identifier* id_node);

  void DelArray(Array* node);
};

class VarEnvExecutor: public Executor {
 public:
  VarEnvExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  void Exec(VarEnvStatement *node);
};

class GlobalAssignmentExecutor: public Executor {
 public:
  GlobalAssignmentExecutor(Executor* parent,
      SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  void Exec(GlobalAssignmentStatement *node);
};

}
}

#endif  // SHPP_STMT_EXECUTOR_H
