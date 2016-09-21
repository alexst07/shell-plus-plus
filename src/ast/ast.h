#ifndef SETTI_AST_H
#define SETTI_AST_H

#include <string>
#include <memory>
#include <vector>
#include <iostream>

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

class AstNode {
 public:
#define DECLARE_TYPE_ENUM(type) k##type,
  enum class NodeType : uint8_t { AST_NODE_LIST(DECLARE_TYPE_ENUM) };
#undef DECLARE_TYPE_ENUM

  virtual ~AstNode() {}

 private:
  NodeType type_;
  int position_;

 protected:
  AstNode(NodeType type, int position):
      type_(type), position_(position) {}
};

class Expression: public AstNode {
 public:
  virtual ~Expression() {}

 protected:
  Expression(NodeType type, int position): AstNode(type, position) {}


};

class BinaryOperation: public Expression {
 public:
  virtual ~BinaryOperation() {}

 private:
  friend class AstNodeFactory;

  TokenKind token_kind_;
  std::unique_ptr<Expression> left_;
  std::unique_ptr<Expression> right_;

  BinaryOperation(TokenKind token_kind, std::unique_ptr<Expression> left,
                  std::unique_ptr<Expression> right, int position)
      : Expression(NodeType::kBinaryOperation, position)
      , token_kind_(token_kind)
      , left_(std::move(left))
      , right_(std::move(right)) {}
};

class Literal: public Expression {
 public:
  virtual ~Literal() {}

 private:
  friend class AstNodeFactory;

  Token token_;

  Literal(const Token& token, int position):
    token_(token), Expression(NodeType::kLiteral, position) {}
};

class AstNodeFactory {
 public:
  inline std::unique_ptr<Literal> NewLiteral(const Token& token, int pos) {
    return std::unique_ptr<Literal>(new Literal(token, pos));
  }

  inline std::unique_ptr<BinaryOperation> NewBinaryOperation(
      TokenKind token_kind, std::unique_ptr<Expression> left,
      std::unique_ptr<Expression> right, int position) {
    return std::unique_ptr<BinaryOperation>(new BinaryOperation(
        token_kind, std::move(left), std::move(right), position));
  }
};

}
}

#endif  // SETTI_AST_H


