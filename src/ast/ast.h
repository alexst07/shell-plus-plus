#ifndef SETTI_AST_H
#define SETTI_AST_H

#include <string>
#include <memory>
#include <vector>
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
  V(ObjectLiteral)           \
  V(ArrayLiteral)

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
  V(NativeFunctionLiteral)      \
  V(Conditional)                \
  V(VariableProxy)              \
  V(Literal)                    \
  V(Yield)                      \
  V(Throw)                      \
  V(CallRuntime)                \
  V(UnaryOperation)             \
  V(BinaryOperation)            \
  V(CompareOperation)           \
  V(Spread)                     \
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
class BinaryOperation;
class Literal;

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
  void virtual VisitBinaryOperation(BinaryOperation* bin_op) {}

  void virtual VisitLiteral(Literal* lit_exp) {}
};

class Expression: public AstNode {
 public:
  virtual ~Expression() {}

  virtual void Accept(AstVisitor* visitor) = 0;

 protected:
  Expression(NodeType type, Position position): AstNode(type, position) {}


};

class BinaryOperation: public Expression {
 public:
  virtual ~BinaryOperation() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitBinaryOperation(this);
  }

  TokenKind token_kind() const noexcept {
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

 private:
  std::function<Position()> fn_pos_;
};

}
}

#endif  // SETTI_AST_H


