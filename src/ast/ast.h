#ifndef SETTI_AST_H
#define SETTI_AST_H

#include <string>
#include <memory>
#include <vector>
#include <list>
#include <iostream>
#include <functional>

#include "parser/token.h"
#include "msg.h"
#include "parser/lexer.h"

namespace setti {
namespace internal {

#define DECLARATION_NODE_LIST(V) \
  V(VariableDeclaration)         \
  V(FunctionDeclaration)

#define ITERATION_NODE_LIST(V) \
  V(DoWhileStatement)          \
  V(WhileStatement)            \
  V(ForStatement)              \
  V(ForInStatement)

#define BREAKABLE_NODE_LIST(V) \
  V(Block)                     \
  V(SwitchStatement)

#define STATEMENT_NODE_LIST(V)    \
  ITERATION_NODE_LIST(V)          \
  BREAKABLE_NODE_LIST(V)          \
  V(StatementList)                \
  V(AssignmentStatement)          \
  V(ExpressionStatement)          \
  V(EmptyStatement)               \
  V(IfStatement)                  \
  V(ContinueStatement)            \
  V(BreakStatement)               \
  V(ReturnStatement)              \
  V(WithStatement)                \
  V(TryCatchStatement)            \
  V(TryFinallyStatement)          \
  V(DebuggerStatement)

#define LITERAL_NODE_LIST(V) \
  V(RegExpLiteral)           \
  V(ObjectLiteral)

#define PROPERTY_NODE_LIST(V) \
  V(Assignment)               \
  V(CountOperation)           \
  V(Property)

#define CALL_NODE_LIST(V) \
  V(Call)                 \
  V(CallNew)

#define EXPRESSION_NODE_LIST(V) \
  LITERAL_NODE_LIST(V)          \
  PROPERTY_NODE_LIST(V)         \
  CALL_NODE_LIST(V)             \
  V(FunctionLiteral)            \
  V(ClassLiteral)               \
  V(Attribute)                  \
  V(Conditional)                \
  V(VariableProxy)              \
  V(Literal)                    \
  V(Array)                      \
  V(Identifier)                 \
  V(Yield)                      \
  V(Throw)                      \
  V(CallRuntime)                \
  V(UnaryOperation)             \
  V(BinaryOperation)            \
  V(CompareOperation)           \
  V(ExpressionList)             \
  V(FunctionCall)               \
  V(ThisFunction)               \
  V(SuperPropertyReference)     \
  V(SuperCallReference)         \
  V(CaseClause)                 \
  V(EmptyParentheses)           \
  V(DoExpression)

#define AST_NODE_LIST(V)        \
  DECLARATION_NODE_LIST(V)      \
  STATEMENT_NODE_LIST(V)        \
  EXPRESSION_NODE_LIST(V)

class AstNodeFactory;

class AstVisitor;
class Expression;
class ExpressionList;
class BinaryOperation;
class Literal;
class Identifier;
class AssignmentStatement;
class UnaryOperation;
class Array;
class Attribute;
class FunctionCall;
class StatementList;
class ExpressionStatement;
class IfStatement;
class Block;

// Position of ast node on source code
struct Position {
  uint line;
  uint col;
};

class AstNode {
 public:
#define DECLARE_TYPE_ENUM(type) k##type,
  enum class NodeType : uint8_t { AST_NODE_LIST(DECLARE_TYPE_ENUM) };
#undef DECLARE_TYPE_ENUM

  virtual ~AstNode() {}

  NodeType type() {
    return type_;
  }

  virtual void Accept(AstVisitor* visitor) = 0;

 private:
  NodeType type_;
  Position position_;

 protected:
  AstNode(NodeType type, Position position):
      type_(type), position_(position) {}
};

class AstVisitor {
 public:
  void virtual VisitExpressionList(ExpressionList *exp_list) {}

  void virtual VisitBinaryOperation(BinaryOperation* bin_op) {}

  void virtual VisitLiteral(Literal* lit_exp) {}

  void virtual VisitIdentifier(Identifier* id) {}

  void virtual VisitAssignmentStatement(AssignmentStatement* assig) {}

  void virtual VisitUnaryOperation(UnaryOperation* un_op) {}

  void virtual VisitArray(Array* arr) {}

  void virtual VisitAttribute(Attribute* attribute) {}

  void virtual VisitFunctionCall(FunctionCall* func) {}

  void virtual VisitStatementList(StatementList* stmt_list) {}

  void virtual VisitExpressionStatement(ExpressionStatement *exp_stmt) {}

  void virtual VisitIfStatement(IfStatement* if_stmt) {}

  void virtual VisitBlock(Block* block) {}
};

class Statement: public AstNode {
 public:
  virtual ~Statement() {}

  virtual void Accept(AstVisitor* visitor) = 0;

 protected:
  Statement(NodeType type, Position position): AstNode(type, position) {}
};

class Expression: public Statement {
 public:
  virtual ~Expression() {}

  virtual void Accept(AstVisitor* visitor) = 0;

 protected:
  Expression(NodeType type, Position position): Statement(type, position) {}
};

class StatementList: public AstNode {
 public:
  virtual ~StatementList() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitStatementList(this);
  }

  bool IsEmpty() const noexcept {
    return stmt_list_.empty();
  }

  std::vector<Statement*> children() noexcept {
    std::vector<Statement*> vec;

    for (auto&& p_stmt: stmt_list_) {
      vec.push_back(p_stmt.get());
    }

    return vec;
  }

  size_t num_children() const noexcept {
    stmt_list_.size();
  }

 private:
  friend class AstNodeFactory;

  std::vector<std::unique_ptr<Statement>> stmt_list_;

  StatementList(std::vector<std::unique_ptr<Statement>> stmt_list,
                Position position)
      : AstNode(NodeType::kStatementList, position)
      , stmt_list_(std::move(stmt_list)) {}
};

class Block: public Statement {
 public:
  virtual ~Block() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitBlock(this);
  }

  StatementList* stmt_list() const noexcept {
    return stmt_list_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<StatementList> stmt_list_;

  Block(std::unique_ptr<StatementList> stmt_list, Position position)
      : Statement(NodeType::kBlock, position)
      , stmt_list_(std::move(stmt_list)) {}
};

class ExpressionList: public AstNode {
 public:
  virtual ~ExpressionList() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitExpressionList(this);
  }

  bool IsEmpty() const noexcept {
    return exps_.empty();
  }

  std::vector<Expression*> children() noexcept {
    std::vector<Expression*> vec;

    for (auto&& p_exp: exps_) {
      vec.push_back(p_exp.get());
    }

    return vec;
  }

  size_t num_children() const noexcept {
    exps_.size();
  }

 private:
  friend class AstNodeFactory;

  std::vector<std::unique_ptr<Expression>> exps_;

  ExpressionList(std::vector<std::unique_ptr<Expression>> exps,
                 Position position)
      : AstNode(NodeType::kExpressionList, position)
      , exps_(std::move(exps)) {}
};

class IfStatement: public Statement {
 public:
  virtual ~IfStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitIfStatement(this);
  }

  Expression* exp() const noexcept {
    return exp_.get();
  }

  Statement* then_block() const noexcept {
    return then_block_.get();
  }

  Statement* else_block() const noexcept {
    return else_block_.get();
  }

  bool has_else() const noexcept {
    if (else_block_) { return true; }

    return false;
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Expression> exp_;
  std::unique_ptr<Statement> then_block_;
  std::unique_ptr<Statement> else_block_;

  IfStatement(std::unique_ptr<Expression> exp,
              std::unique_ptr<Statement> then_block,
              std::unique_ptr<Statement> else_block,
              Position position)
      : Statement(NodeType::kIfStatement, position)
      , exp_(std::move(exp))
      , then_block_(std::move(then_block))
      , else_block_(std::move(else_block)) {}
};

class AssignmentStatement: public Statement {
 public:
  virtual ~AssignmentStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitAssignmentStatement(this);
  }

  TokenKind assign_kind() const noexcept {
    return assign_kind_;
  }

  ExpressionList* lexp_list() const noexcept {
    return lexp_.get();
  }

  ExpressionList* rexp_list() const noexcept {
    return rexp_.get();
  }

 private:
  friend class AstNodeFactory;

  TokenKind assign_kind_;
  std::unique_ptr<ExpressionList> lexp_;
  std::unique_ptr<ExpressionList> rexp_;

  AssignmentStatement(TokenKind assign_kind,
                      std::unique_ptr<ExpressionList> lexp,
                      std::unique_ptr<ExpressionList> rexp_,
                      Position position)
      : Statement(NodeType::kAssignmentStatement, position)
      , assign_kind_(assign_kind)
      , lexp_(std::move(lexp))
      , rexp_(std::move(rexp_)) {}
};

class ExpressionStatement: public Statement {
 public:
  virtual ~ExpressionStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitExpressionStatement(this);
  }

  Expression* exp() const noexcept {
    return exp_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Expression> exp_;

  ExpressionStatement(std::unique_ptr<Expression> exp, Position position)
      : Statement(NodeType::kExpressionStatement, position)
      , exp_(std::move(exp)) {}
};


class BinaryOperation: public Expression {
 public:
  virtual ~BinaryOperation() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitBinaryOperation(this);
  }

  TokenKind kind() const noexcept {
    return token_kind_;
  }

  Expression* left() const noexcept {
    return left_.get();
  }

  Expression* right() const noexcept {
    return right_.get();
  }

 private:
  friend class AstNodeFactory;

  TokenKind token_kind_;
  std::unique_ptr<Expression> left_;
  std::unique_ptr<Expression> right_;

  BinaryOperation(TokenKind token_kind, std::unique_ptr<Expression> left,
                  std::unique_ptr<Expression> right, Position position)
      : Expression(NodeType::kBinaryOperation, position)
      , token_kind_(token_kind)
      , left_(std::move(left))
      , right_(std::move(right)) {}
};

class Identifier: public Expression {
 public:
  virtual ~Identifier() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitIdentifier(this);
  }

  const std::string& name() const noexcept {
    return name_;
  }

 private:
  friend class AstNodeFactory;

  std::string name_;

  Identifier(const std::string& name, Position position):
    name_(name), Expression(NodeType::kIdentifier, position) {}
};

class UnaryOperation: public Expression {
 public:
  virtual ~UnaryOperation() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitUnaryOperation(this);
  }

  TokenKind kind() const noexcept {
    return token_kind_;
  }

  Expression* exp() const noexcept {
    return exp_.get();
  }

 private:
  friend class AstNodeFactory;

  TokenKind token_kind_;
  std::unique_ptr<Expression> exp_;

  UnaryOperation(TokenKind token_kind, std::unique_ptr<Expression> exp,
                 Position position)
      : Expression(NodeType::kUnaryOperation, position)
      , token_kind_(token_kind)
      , exp_(std::move(exp)) {}
};

class Array: public Expression {
 public:
  virtual ~Array() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitArray(this);
  }

  Expression* index_exp() const noexcept {
    return index_exp_.get();
  }

  Expression* arr_exp() const noexcept {
    return arr_exp_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Expression> index_exp_;
  std::unique_ptr<Expression> arr_exp_;

  Array(std::unique_ptr<Expression> arr_exp,
        std::unique_ptr<Expression> index_exp, Position position)
      : Expression(NodeType::kArray, position)
      , index_exp_(std::move(index_exp))
      , arr_exp_(std::move(arr_exp)) {}
};

class Attribute: public Expression {
 public:
  virtual ~Attribute() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitAttribute(this);
  }

  Expression* exp() const noexcept {
    return exp_.get();
  }

  Identifier* id() const noexcept {
    return id_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Expression> exp_;
  std::unique_ptr<Identifier> id_;

  Attribute(std::unique_ptr<Expression> exp, std::unique_ptr<Identifier> id,
            Position position)
      : Expression(NodeType::kAttribute, position)
      , exp_(std::move(exp))
      , id_(std::move(id)) {}
};

class FunctionCall: public Expression {
 public:
  virtual ~FunctionCall() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitFunctionCall(this);
  }

  Expression* func_exp() {
    return func_exp_.get();
  }

  bool IsListExpEmpty() const noexcept {
    return exp_list_->IsEmpty();
  }

  ExpressionList* exp_list() {
    return exp_list_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Expression> func_exp_;
  std::unique_ptr<ExpressionList> exp_list_;

  FunctionCall(std::unique_ptr<Expression> func_exp,
               std::unique_ptr<ExpressionList> exp_list, Position position)
      : Expression(NodeType::kFunctionCall, position)
      , func_exp_(std::move(func_exp))
      , exp_list_(std::move(exp_list)) {}
};

class Literal: public Expression {
 public:
  enum Type {
    kString,
    kInteger,
    kReal,
    kBool
  };

  virtual ~Literal() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitLiteral(this);
  }

  const Token::Value& value() const noexcept {
    return value_;
  }

 private:
  friend class AstNodeFactory;

  Token::Value value_;

  Literal(const Token::Value& value, Type type, Position position):
    value_(value), Expression(NodeType::kLiteral, position) {}
};

class AstNodeFactory {
 public:
  AstNodeFactory(const std::function<Position()> fn_pos): fn_pos_(fn_pos) {}

  inline std::unique_ptr<Literal> NewLiteral(const Token::Value& value,
                                             Literal::Type type) {
    return std::unique_ptr<Literal>(new Literal(value, type, fn_pos_()));
  }

  inline std::unique_ptr<BinaryOperation> NewBinaryOperation(
      TokenKind token_kind, std::unique_ptr<Expression> left,
      std::unique_ptr<Expression> right) {
    return std::unique_ptr<BinaryOperation>(new BinaryOperation(
        token_kind, std::move(left), std::move(right), fn_pos_()));
  }

  inline std::unique_ptr<UnaryOperation> NewUnaryOperation(
      TokenKind token_kind, std::unique_ptr<Expression> exp) {
    return std::unique_ptr<UnaryOperation>(new UnaryOperation(
        token_kind, std::move(exp), fn_pos_()));
  }

  inline std::unique_ptr<Array> NewArray(std::unique_ptr<Expression> arr_exp,
                                         std::unique_ptr<Expression> index_exp) {
    return std::unique_ptr<Array>(new Array(std::move(arr_exp),
                                            std::move(index_exp), fn_pos_()));
  }

  inline std::unique_ptr<Attribute> NewAttribute(
      std::unique_ptr<Expression> exp, std::unique_ptr<Identifier> id) {
    return std::unique_ptr<Attribute>(new Attribute(std::move(exp),
                                                    std::move(id),
                                                    fn_pos_()));
  }

  inline std::unique_ptr<Identifier> NewIdentifier(const std::string& name) {
    return std::unique_ptr<Identifier>(new Identifier(name, fn_pos_()));
  }

  inline std::unique_ptr<AssignmentStatement> NewAssignmentStatement(
      TokenKind assign_kind, std::unique_ptr<ExpressionList> lexp_list,
      std::unique_ptr<ExpressionList> rexp_list) {
    return std::unique_ptr<AssignmentStatement>(new AssignmentStatement(
        assign_kind, std::move(lexp_list), std::move(rexp_list), fn_pos_()));
  }

  inline std::unique_ptr<ExpressionList> NewExpressionList(
      std::vector<std::unique_ptr<Expression>> exps) {
    return std::unique_ptr<ExpressionList>(new ExpressionList(std::move(exps),
                                                              fn_pos_()));
  }

  inline std::unique_ptr<StatementList> NewStatementList(
      std::vector<std::unique_ptr<Statement>> stmt_list) {
    return std::unique_ptr<StatementList>(new StatementList(
        std::move(stmt_list), fn_pos_()));
  }

  inline std::unique_ptr<FunctionCall> NewFunctionCall(
      std::unique_ptr<Expression> func_exp,
      std::unique_ptr<ExpressionList> exp_list) {
    return std::unique_ptr<FunctionCall>(new FunctionCall(
        std::move(func_exp), std::move(exp_list), fn_pos_()));
  }

  inline std::unique_ptr<ExpressionStatement> NewExpressionStatement(
      std::unique_ptr<Expression> exp_stmt) {
    return std::unique_ptr<ExpressionStatement>(new ExpressionStatement(
        std::move(exp_stmt), fn_pos_()));
  }

  inline std::unique_ptr<Statement> NewBlock(
      std::unique_ptr<StatementList> stmt_list) {
    return std::unique_ptr<Statement>(new Block(
        std::move(stmt_list), fn_pos_()));
  }

  inline std::unique_ptr<IfStatement> NewIfStatement(
      std::unique_ptr<Expression> exp,
      std::unique_ptr<Statement> then_block,
      std::unique_ptr<Statement> else_block) {
    return std::unique_ptr<IfStatement>(new IfStatement(
        std::move(exp), std::move(then_block), std::move(else_block),
        fn_pos_()));
  }

 private:
  std::function<Position()> fn_pos_;
};

}
}

#endif  // SETTI_AST_H


