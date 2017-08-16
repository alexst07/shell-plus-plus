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

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Identifier> name_;

  FunctionDeclaration(std::vector<std::unique_ptr<FunctionParam>>&& params,
                      std::unique_ptr<Identifier> name,
                      std::shared_ptr<Block> block, Position position)
    : Declaration(NodeType::kFunctionDeclaration, position)
    , Function(std::move(params), block)
    , name_(std::move(name)) {}
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

  std::vector<Declaration*> children() noexcept {
    std::vector<Declaration*> vec;

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

  std::vector<std::unique_ptr<Declaration>> decl_list_;

  ClassDeclList(std::vector<std::unique_ptr<Declaration>> decl_list,
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

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Identifier> name_;
  std::unique_ptr<Expression> parent_;
  std::unique_ptr<ExpressionList> interfaces_;
  std::unique_ptr<ClassBlock> block_;
  bool is_final_;

  ClassDeclaration(std::unique_ptr<Identifier> name,
                   std::unique_ptr<Expression> parent,
                   std::unique_ptr<ExpressionList> interfaces,
                   std::unique_ptr<ClassBlock> block, bool is_final,
                   Position position)
      : Declaration(NodeType::kClassDeclaration, position)
      , name_(std::move(name))
      , parent_(std::move(parent))
      , interfaces_(std::move(interfaces))
      , block_(std::move(block))
      , is_final_(is_final) {}
};

}
}
