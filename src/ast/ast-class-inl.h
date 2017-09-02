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

#ifndef SHPP_AST_H
#error This file may only be included from ast.h.
#endif

namespace shpp {
namespace internal {

class CmdDeclaration: public Declaration {
 public:
  virtual ~CmdDeclaration() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitCmdDeclaration(this);
  }

  Identifier* id() const noexcept {
    return id_.get();
  }

  std::shared_ptr<Block> block() const noexcept {
    return block_;
  }

 private:
  friend class AstNodeFactory;

  std::shared_ptr<Block> block_;
  std::unique_ptr<Identifier> id_;

  CmdDeclaration(std::unique_ptr<Identifier> id, std::unique_ptr<Block> block,
                Position position)
      : Declaration(NodeType::kCmdDeclaration, position)
      , block_(std::move(block))
      , id_(std::move(id)) {}
};

class Function {
 public:
  virtual ~Function() {}

  bool variadic() const noexcept {
   if (params_.empty()) {
     return false;
   }

   return params_.back()->variadic();
  }

  std::vector<FunctionParam*> children() noexcept {
    std::vector<FunctionParam*> vec;

    for (auto& p_exp: params_) {
      vec.push_back(p_exp.get());
    }

    return vec;
  }

  const std::vector<std::unique_ptr<FunctionParam>>& params() const noexcept {
    return params_;
  }

  // shared is passed because the pointer of body function
  // is used by others objects, this object can exists
  // even when the ast doesn't exists anymore, it happens
  // when is using interactive mode
  std::shared_ptr<Block> block() const noexcept {
    return block_;
  }

  bool has_block() const noexcept {
    if (block_) {
      return true;
    }

    return false;
  }

 private:
  friend class AstNodeFactory;

  std::vector<std::unique_ptr<FunctionParam>> params_;
  std::shared_ptr<Block> block_;

 protected:
  Function(std::vector<std::unique_ptr<FunctionParam>>&& params,
           std::shared_ptr<Block> block)
    : params_(std::move(params))
    , block_(std::move(block)) {}
};

class FunctionDeclaration: public Function, public Declaration {
 public:
  virtual ~FunctionDeclaration() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitFunctionDeclaration(this);
  }

  Identifier* name() const noexcept {
    return name_.get();
  }

  bool fstatic() const noexcept {
    return fstatic_;
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Identifier> name_;
  bool fstatic_;

  FunctionDeclaration(std::vector<std::unique_ptr<FunctionParam>>&& params,
                      std::unique_ptr<Identifier> name,
                      std::shared_ptr<Block> block,
                      bool fstatic,
                      Position position)
    : Declaration(NodeType::kFunctionDeclaration, position)
    , Function(std::move(params), block)
    , name_(std::move(name))
    , fstatic_(fstatic) {}
};

class FunctionExpression: public Function, public Expression {
 public:
  virtual ~FunctionExpression() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitFunctionExpression(this);
  }

 private:
  friend class AstNodeFactory;

  FunctionExpression(std::vector<std::unique_ptr<FunctionParam>>&& params,
                     std::shared_ptr<Block> block, Position position)
    : Expression(NodeType::kFunctionExpression, position)
    , Function(std::move(params), block) {}
};

class ClassDeclList: public AstNode {
 public:
  virtual ~ClassDeclList() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitClassDeclList(this);
  }

  bool IsEmpty() const noexcept {
    return decl_list_.empty();
  }

  std::vector<AstNode*> children() noexcept {
    std::vector<AstNode*> vec;

    for (auto&& p_decl: decl_list_) {
      vec.push_back(p_decl.get());
    }

    return vec;
  }

  size_t num_children() const noexcept {
    return decl_list_.size();
  }

 private:
  friend class AstNodeFactory;

  std::vector<std::unique_ptr<AstNode>> decl_list_;

  ClassDeclList(std::vector<std::unique_ptr<AstNode>> decl_list,
                Position position)
      : AstNode(NodeType::kClassDeclList, position)
      , decl_list_(std::move(decl_list)) {}
};

class ClassBlock: public AstNode {
 public:
  virtual ~ClassBlock() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitClassBlock(this);
  }

  ClassDeclList* decl_list() const noexcept {
    return decl_list_.get();
  }

  bool is_empty() const noexcept {
    if (decl_list_->num_children() == 0) {
      return true;
    }

    return false;
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<ClassDeclList> decl_list_;

  ClassBlock(std::unique_ptr<ClassDeclList> decl_list, Position position)
      : AstNode(NodeType::kClassBlock, position)
      , decl_list_(std::move(decl_list)) {}
};

class ClassDeclaration: public Declaration {
 public:
  virtual ~ClassDeclaration() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitClassDeclaration(this);
  }

  Expression* parent() const noexcept {
    return parent_.get();
  }

  bool has_parent() const noexcept {
    if (parent_) {
      return true;
    }

    return false;
  }

  ClassBlock* block() const noexcept {
    return block_.get();
  }

  bool has_block() const noexcept {
    if (block_) {
      return true;
    }

    return false;
  }

  Identifier* name() const noexcept {
    return name_.get();
  }

  bool is_final() const noexcept {
    return is_final_;
  }

  bool has_interfaces() const noexcept {
    if (interfaces_) {
      return true;
    }

    return false;
  }

  ExpressionList* interfaces() noexcept {
    return interfaces_.get();
  }

  bool abstract() const noexcept {
    return abstract_;
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Identifier> name_;
  std::unique_ptr<Expression> parent_;
  std::unique_ptr<ExpressionList> interfaces_;
  std::unique_ptr<ClassBlock> block_;
  bool is_final_;
  bool abstract_;

  ClassDeclaration(std::unique_ptr<Identifier> name,
                   std::unique_ptr<Expression> parent,
                   std::unique_ptr<ExpressionList> interfaces,
                   std::unique_ptr<ClassBlock> block,
                   bool is_final,
                   bool abstract,
                   Position position)
      : Declaration(NodeType::kClassDeclaration, position)
      , name_(std::move(name))
      , parent_(std::move(parent))
      , interfaces_(std::move(interfaces))
      , block_(std::move(block))
      , is_final_(is_final)
      , abstract_(abstract) {}
};

class InterfaceDeclList: public AstNode {
 public:
  virtual ~InterfaceDeclList() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitInterfaceDeclList(this);
  }

  bool IsEmpty() const noexcept {
    return decl_list_.empty();
  }

  std::vector<AstNode*> children() noexcept {
    std::vector<AstNode*> vec;

    for (auto&& p_decl: decl_list_) {
      vec.push_back(p_decl.get());
    }

    return vec;
  }

  size_t num_children() const noexcept {
    return decl_list_.size();
  }

 private:
  friend class AstNodeFactory;

  std::vector<std::unique_ptr<AstNode>> decl_list_;

  InterfaceDeclList(
      std::vector<std::unique_ptr<AstNode>> decl_list,
      Position position)
      : AstNode(NodeType::kInterfaceDeclList, position)
      , decl_list_(std::move(decl_list)) {}
};

class InterfaceBlock: public AstNode {
 public:
  virtual ~InterfaceBlock() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitInterfaceBlock(this);
  }

  InterfaceDeclList* decl_list() const noexcept {
    return decl_list_.get();
  }

  bool is_empty() const noexcept {
    if (decl_list_->num_children() == 0) {
      return true;
    }

    return false;
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<InterfaceDeclList> decl_list_;

  InterfaceBlock(std::unique_ptr<InterfaceDeclList> decl_list,
      Position position)
      : AstNode(NodeType::kInterfaceBlock, position)
      , decl_list_(std::move(decl_list)) {}
};

class InterfaceDeclaration: public Declaration {
 public:
  virtual ~InterfaceDeclaration() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitInterfaceDeclaration(this);
  }

  InterfaceBlock* block() const noexcept {
    return block_.get();
  }

  Identifier* name() const noexcept {
    return name_.get();
  }

  bool has_interfaces() const noexcept {
    if (interfaces_) {
      return true;
    }

    return false;
  }

  ExpressionList* interfaces() noexcept {
    return interfaces_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Identifier> name_;
  std::unique_ptr<ExpressionList> interfaces_;
  std::unique_ptr<InterfaceBlock> block_;

  InterfaceDeclaration(std::unique_ptr<Identifier> name,
      std::unique_ptr<ExpressionList> interfaces,
      std::unique_ptr<InterfaceBlock> block,
      Position position)
      : Declaration(NodeType::kInterfaceDeclaration, position)
      , name_(std::move(name))
      , interfaces_(std::move(interfaces))
      , block_(std::move(block)) {}
};

class VariableDeclaration: public Declaration {
 public:
  virtual ~VariableDeclaration() {}

  virtual void Accept(AstVisitor* visitor) {
   visitor->VisitVariableDeclaration(this);
  }

  Identifier* name() const noexcept {
    return name_.get();
  }

  AssignableValue* value() const noexcept {
    return value_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Identifier> name_;
  std::unique_ptr<AssignableValue> value_;

  VariableDeclaration(std::unique_ptr<Identifier> name,
      std::unique_ptr<AssignableValue> value,
      Position position)
      : Declaration(NodeType::kVariableDeclaration, position)
      , name_(std::move(name))
      , value_(std::move(value)) {}
};

class CatchStatement: public Statement {
 public:
  virtual ~CatchStatement() {}

  virtual void Accept(AstVisitor* visitor) {
   visitor->VisitCatchStatement(this);
  }

  Identifier* var() const noexcept {
    return var_.get();
  }

  bool has_var() const noexcept {
    if (var_)  {
      return true;
    }

    return false;
  }

  ExpressionList* exp_list() const noexcept {
    return exp_list_.get();
  }

  Block* block() const noexcept {
    return block_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<ExpressionList> exp_list_;
  std::unique_ptr<Block> block_;
  std::unique_ptr<Identifier> var_;

  CatchStatement(std::unique_ptr<ExpressionList> exp_list,
      std::unique_ptr<Block> block, std::unique_ptr<Identifier> var,
      Position position)
      : Statement(NodeType::kCatchStatement, position)
      , exp_list_(std::move(exp_list))
      , block_(std::move(block))
      , var_(std::move(var)) {}
};

class FinallyStatement: public Statement {
 public:
  virtual ~FinallyStatement() {}

  virtual void Accept(AstVisitor* visitor) {
   visitor->VisitFinallyStatement(this);
  }

  Block* block() const noexcept {
    return block_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Block> block_;

  FinallyStatement(std::unique_ptr<Block> block, Position position)
      : Statement(NodeType::kFinallyStatement, position)
      , block_(std::move(block)) {}
};

class TryCatchStatement: public Statement {
 public:
  virtual ~TryCatchStatement() {}

  virtual void Accept(AstVisitor* visitor) {
   visitor->VisitTryCatchStatement(this);
  }

  Block* try_block() const noexcept {
    return try_block_.get();
  }

  std::vector<CatchStatement*> catch_list() const noexcept {
    std::vector<CatchStatement*> vec;

    for (auto& piece: catch_list_) {
      vec.push_back(piece.get());
    }

    return vec;
  }

  bool has_catch() const noexcept {
    if (catch_list_.size() > 0) {
      return true;
    }

    return false;
  }

  FinallyStatement* finally() const noexcept {
    return finally_.get();
  }

  bool has_finally() const noexcept {
    if (finally_) {
      return true;
    }

    return false;
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Block> try_block_;
  std::vector<std::unique_ptr<CatchStatement>> catch_list_;
  std::unique_ptr<FinallyStatement> finally_;

  TryCatchStatement(std::unique_ptr<Block> try_block,
      std::vector<std::unique_ptr<CatchStatement>> catch_list,
      std::unique_ptr<FinallyStatement> finally, Position position)
      : Statement(NodeType::kTryCatchStatement, position)
      , try_block_(std::move(try_block))
      , catch_list_(std::move(catch_list))
      , finally_(std::move(finally)) {}
};

class ThrowStatement: public Statement {
 public:
  virtual ~ThrowStatement() {}

  virtual void Accept(AstVisitor* visitor) {
   visitor->VisitThrowStatement(this);
  }

  Expression* exp() const noexcept {
    return exp_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Expression> exp_;

  ThrowStatement(std::unique_ptr<Expression> exp, Position position)
      : Statement(NodeType::kThrowStatement, position)
      , exp_(std::move(exp)) {}
};

}
}
