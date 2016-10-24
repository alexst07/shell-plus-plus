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
  V(FunctionDeclaration)         \
  V(FunctionParam)               \
  V(CmdDeclaration)

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
  V(CaseStatement)                \
  V(DefaultStatement)             \
  V(TryCatchStatement)            \
  V(TryFinallyStatement)          \
  V(DebuggerStatement)

#define LITERAL_NODE_LIST(V) \
  V(RegExpLiteral)           \
  V(ObjectLiteral)

#define PROPERTY_NODE_LIST(V) \
  V(Assignment)               \
  V(CountOperation)           \
  V(Property)                 \
  V(AssignableValue)          \
  V(AssignableList)           \
  V(KeyValue)

#define CALL_NODE_LIST(V) \
  V(Call)                 \
  V(CallNew)

#define PAKCAGE_NODE_LIST(V) \
  V(PackageScope)

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
  V(ArrayInstantiation)         \
  V(DictionaryInstantiation)    \
  V(Identifier)                 \
  V(Yield)                      \
  V(Throw)                      \
  V(CallRuntime)                \
  V(UnaryOperation)             \
  V(BinaryOperation)            \
  V(CompareOperation)           \
  V(ExpressionList)             \
  V(FunctionCall)               \
  V(CmdExpression)              \
  V(ThisFunction)               \
  V(SuperPropertyReference)     \
  V(SuperCallReference)         \
  V(CaseClause)                 \
  V(EmptyParentheses)           \
  V(DoExpression)

#define CMD_NODE_LIST(V)   \
  V(Cmd)                   \
  V(CmdPiece)              \
  V(SimpleCmd)             \
  V(CmdIoRedirect)         \
  V(FilePathCmd)           \
  V(CmdIoRedirectList)     \
  V(CmdPipeSequence)       \
  V(CmdAndOr)              \
  V(CmdFull)               \
  V(SubShell)

#define AST_NODE_LIST(V)        \
  DECLARATION_NODE_LIST(V)      \
  STATEMENT_NODE_LIST(V)        \
  EXPRESSION_NODE_LIST(V)       \
  CMD_NODE_LIST(V)              \
  PAKCAGE_NODE_LIST(V)

class AstNodeFactory;

class AstVisitor;
class Expression;
class ExpressionList;
class BinaryOperation;
class Literal;
class Identifier;
class PackageScope;
class AssignmentStatement;
class UnaryOperation;
class Array;
class Attribute;
class FunctionCall;
class StatementList;
class ExpressionStatement;
class IfStatement;
class Block;
class WhileStatement;
class BreakStatement;
class CaseStatement;
class DefaultStatement;
class SwitchStatement;
class ForInStatement;
class Cmd;
class CmdPiece;
class SimpleCmd;
class CmdIoRedirect;
class CmdIoRedirectList;
class FilePathCmd;
class CmdPipeSequence;
class CmdAndOr;
class CmdFull;
class SubShell;
class CmdExpression;
class FunctionParam;
class FunctionDeclaration;
class ArrayInstantiation;
class AssignableValue;
class AssignableList;
class KeyValue;
class DictionaryInstantiation;
class ReturnStatement;
class CmdDeclaration;

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

  void virtual VisitPackageScope(PackageScope* scope) {}

  void virtual VisitAssignmentStatement(AssignmentStatement* assig) {}

  void virtual VisitUnaryOperation(UnaryOperation* un_op) {}

  void virtual VisitArray(Array* arr) {}

  void virtual VisitAttribute(Attribute* attribute) {}

  void virtual VisitFunctionCall(FunctionCall* func) {}

  void virtual VisitStatementList(StatementList* stmt_list) {}

  void virtual VisitExpressionStatement(ExpressionStatement *exp_stmt) {}

  void virtual VisitIfStatement(IfStatement* if_stmt) {}

  void virtual VisitBlock(Block* block) {}

  void virtual VisitWhileStatement(WhileStatement* while_stmt) {}

  void virtual VisitBreakStatement(BreakStatement* pbreak) {}

  void virtual VisitCaseStatement(CaseStatement* case_stmt) {}

  void virtual VisitDefaultStatement(DefaultStatement* default_stmt) {}

  void virtual VisitSwitchStatement(SwitchStatement* switch_stmt) {}

  void virtual VisitForInStatement(ForInStatement* for_in_stmt) {}

  void virtual VisitCmdPiece(CmdPiece* cmd_piece) {}

  void virtual VisitSimpleCmd(SimpleCmd *cmd) {}

  void virtual VisitCmdIoRedirect(CmdIoRedirect* io) {}

  void virtual VisitCmdIoRedirectList(CmdIoRedirectList *io_list) {}

  void virtual VisitFilePathCmd(FilePathCmd* fp_cmd) {}

  void virtual VisitCmdPipeSequence(CmdPipeSequence* cmd_pipe) {}

  void virtual VisitCmdAndOr(CmdAndOr* cmd_and_or) {}

  void virtual VisitCmdFull(CmdFull* cmd_full) {}

  void virtual VisitCmdExpression(CmdExpression* cmd) {}

  void virtual VisitFunctionParam(FunctionParam* func_param) {}

  void virtual VisitFunctionDeclaration(FunctionDeclaration* func_decl) {}

  void virtual VisitArrayInstantiation(ArrayInstantiation* array) {}

  void virtual VisitAssignableValue(AssignableValue* value) {}

  void virtual VisitAssignableList(AssignableList* assign_list) {}

  void virtual VisitKeyValue(KeyValue* key_value) {}

  void virtual VisitDictionaryInstantiation(DictionaryInstantiation* dic) {}

  void virtual VisitReturnStatement(ReturnStatement* ret) {}

  void virtual VisitCmdDeclaration(CmdDeclaration* cmd_decl) {}

  void virtual VisitSubShell(SubShell* sub_shell) {}
};

class Statement: public AstNode {
 public:
  virtual ~Statement() {}

  virtual void Accept(AstVisitor* visitor) = 0;

 protected:
  Statement(NodeType type, Position position): AstNode(type, position) {}
};

class Declaration: public Statement {
 public:
  virtual ~Declaration() {}

  virtual void Accept(AstVisitor* visitor) = 0;

 protected:
  Declaration(NodeType type, Position position): Statement(type, position) {}
};

// Interface class to assignable
class AssignableInterface {

};

class Expression: public Statement, public AssignableInterface {
 public:
  virtual ~Expression() {}

  virtual void Accept(AstVisitor* visitor) = 0;

 protected:
  Expression(NodeType type, Position position): Statement(type, position) {}
};

class Cmd: public Statement {
 public:
  virtual ~Cmd() {}

  virtual void Accept(AstVisitor* visitor) = 0;

 protected:
  Cmd(NodeType type, Position position): Statement(type, position) {}
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

class KeyValue: public AstNode {
 public:
  virtual ~KeyValue() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitKeyValue(this);
  }

  Expression* key() const noexcept {
    return key_.get();
  }

  AssignableValue* value() const noexcept {
    return value_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<AssignableValue> value_;
  std::unique_ptr<Expression> key_;

  KeyValue(std::unique_ptr<Expression> key,
           std::unique_ptr<AssignableValue> value,
           Position position)
      : AstNode(NodeType::kKeyValue, position)
      , key_(std::move(key))
      , value_(std::move(value)) {}
};

class DictionaryInstantiation: public Expression {
 public:
  virtual ~DictionaryInstantiation() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitDictionaryInstantiation(this);
  }

  std::vector<KeyValue*> children() noexcept {
    std::vector<KeyValue*> vec;

    for (auto&& e: key_value_list_) {
      vec.push_back(e.get());
    }

    return vec;
  }

 private:
  friend class AstNodeFactory;

  std::vector<std::unique_ptr<KeyValue>> key_value_list_;

  DictionaryInstantiation(
      std::vector<std::unique_ptr<KeyValue>>&& key_value_list,
      Position position)
      : Expression(NodeType::kDictionaryInstantiation, position)
      , key_value_list_(std::move(key_value_list)) {}
};

class ArrayInstantiation: public Expression {
 public:
  virtual ~ArrayInstantiation() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitArrayInstantiation(this);
  }

  AssignableList* assignable_list() {
    return elements_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<AssignableList> elements_;

  ArrayInstantiation(std::unique_ptr<AssignableList> elements,
                     Position position)
      : Expression(NodeType::kArrayInstantiation, position)
      , elements_(std::move(elements)) {}
};

class AssignableValue: public AstNode, public AssignableInterface {
 public:
  virtual ~AssignableValue() {}

  virtual void Accept(AstVisitor* visitor) {
   visitor->VisitAssignableValue(this);
  }

  AstNode* value() const noexcept {
    return value_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<AstNode> value_;

 template<class T>
 AssignableValue(std::unique_ptr<T>&& value, Position position)
     : AstNode(NodeType::kAssignableValue, position) {
   static_assert(std::is_base_of<AstNode,T>::value,
                 "Type is not derivated from AstNode");

   static_assert(std::is_base_of<AssignableInterface,T>::value,
                 "Type not implements AssignableInterface");

   value_ =  std::move(std::unique_ptr<AstNode>(
         static_cast<AstNode*>(value.release())));
 }
};

class AssignableList: public AstNode, public AssignableInterface {
 public:
  virtual ~AssignableList() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitAssignableList(this);
  }

  std::vector<AssignableValue*> children() noexcept {
    std::vector<AssignableValue*> vec;

    for (auto&& node: nodes_) {
      vec.push_back(node.get());
    }

    return vec;
  }

 private:
  friend class AstNodeFactory;

  std::vector<std::unique_ptr<AssignableValue>> nodes_;

  AssignableList(std::vector<std::unique_ptr<AssignableValue>>&& nodes,
                 Position position)
      : AstNode(NodeType::kAssignableList, position) {
    for (auto&& node: nodes) {
      nodes_.push_back(std::move(node));
    }
  }
};

class FunctionParam: public AstNode {
 public:
  virtual ~FunctionParam() {}

  virtual void Accept(AstVisitor* visitor) {
   visitor->VisitFunctionParam(this);
  }

  bool variadic() const noexcept {
    return variadic_;
  }

  Identifier* id() const noexcept {
    return id_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Identifier> id_;
  bool variadic_;

  FunctionParam(std::unique_ptr<Identifier> id, bool variadic,
                Position position)
     : AstNode(NodeType::kFunctionParam, position)
     , id_(std::move(id))
     , variadic_(variadic) {}
};

class FunctionDeclaration: public Declaration, public AssignableInterface {
 public:
  virtual ~FunctionDeclaration() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitFunctionDeclaration(this);
  }

  bool variadic() const noexcept {
   if (params_.empty()) {
     return false;
   }

   return params_.back()->variadic();
  }

  Identifier* name() const noexcept {
    return name_.get();
  }

  bool is_anonymous() const noexcept {
    if (name_) {
      return false;
    }

    return true;
  }

  std::vector<FunctionParam*> children() noexcept {
    std::vector<FunctionParam*> vec;

    for (auto& p_exp: params_) {
      vec.push_back(p_exp.get());
    }

    return vec;
  }

  Block* block() const noexcept {
    return block_.get();
  }

 private:
  friend class AstNodeFactory;

  std::vector<std::unique_ptr<FunctionParam>> params_;
  std::unique_ptr<Identifier> name_;
  std::unique_ptr<Block> block_;

  FunctionDeclaration(std::vector<std::unique_ptr<FunctionParam>>&& params,
                      std::unique_ptr<Identifier> name,
                      std::unique_ptr<Block> block, Position position)
    : Declaration(NodeType::kFunctionDeclaration, position)
    , name_(std::move(name))
    , params_(std::move(params))
    , block_(std::move(block)) {}
};

class CmdDeclaration: public Declaration {
 public:
  virtual ~CmdDeclaration() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitCmdDeclaration(this);
  }

  Identifier* id() const noexcept {
    return id_.get();
  }

  Block* block() const noexcept {
    return block_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Block> block_;
  std::unique_ptr<Identifier> id_;

  CmdDeclaration(std::unique_ptr<Identifier> id, std::unique_ptr<Block> block,
                Position position)
      : Declaration(NodeType::kCmdDeclaration, position)
      , block_(std::move(block))
      , id_(std::move(id)) {}
};

class ReturnStatement: public Statement {
 public:
  virtual ~ReturnStatement() {}

  virtual void Accept(AstVisitor* visitor) {
   visitor->VisitReturnStatement(this);
  }

  AssignableList* assign_list() const noexcept {
    return assign_list_.get();
  }

  bool is_void() const noexcept {
    if (assign_list_) {
      return false;
    }

    return true;
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<AssignableList> assign_list_;

  ReturnStatement(std::unique_ptr<AssignableList> assign_list,
                  Position position)
      : Statement(NodeType::kReturnStatement, position)
      , assign_list_(std::move(assign_list)) {}
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

class CmdExpression: public Expression {
 public:
  virtual ~CmdExpression() {}

  virtual void Accept(AstVisitor* visitor) {
  visitor->VisitCmdExpression(this);
  }

  Cmd* cmd() const noexcept {
  return cmd_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Cmd> cmd_;

  CmdExpression(std::unique_ptr<Cmd> cmd, Position position)
    : Expression(NodeType::kCmdExpression, position)
    , cmd_(std::move(cmd)) {}
};

class SubShell: public Cmd {
 public:
  virtual ~SubShell() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitSubShell(this);
  }

 Block* block() const noexcept {
  return block_.get();
 }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Block> block_;

  SubShell(std::unique_ptr<Block> block,  Position position)
      : Cmd(NodeType::kCmdFull, position)
      , block_(std::move(block)) {}
};

class CmdFull: public Cmd {
 public:
  virtual ~CmdFull() {}

  virtual void Accept(AstVisitor* visitor) {
   visitor->VisitCmdFull(this);
  }

  bool background() const noexcept {
   return background_;
  }

  Cmd* cmd() const noexcept {
   return cmd_.get();
  }

 private:
  friend class AstNodeFactory;

  bool background_;
  std::unique_ptr<Cmd> cmd_;

  CmdFull(std::unique_ptr<Cmd> cmd, bool background,
          Position position)
     : Cmd(NodeType::kCmdFull, position)
     , cmd_(std::move(cmd))
     , background_(background) {}
};

class CmdAndOr: public Cmd {
 public:
  virtual ~CmdAndOr() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitCmdAndOr(this);
  }

  TokenKind kind() const noexcept {
    return token_kind_;
  }

  Cmd* cmd_left() const noexcept {
    return cmd_left_.get();
  }

  Cmd* cmd_right() const noexcept {
    return cmd_right_.get();
  }

 private:
  friend class AstNodeFactory;

  TokenKind token_kind_;
  std::unique_ptr<Cmd> cmd_left_;
  std::unique_ptr<Cmd> cmd_right_;

  CmdAndOr(TokenKind token_kind, std::unique_ptr<Cmd> cmd_left,
                  std::unique_ptr<Cmd> cmd_right, Position position)
      : Cmd(NodeType::kCmdAndOr, position)
      , token_kind_(token_kind)
      , cmd_left_(std::move(cmd_left))
      , cmd_right_(std::move(cmd_right)) {}
};

class CmdPipeSequence: public Cmd {
 public:
  ~CmdPipeSequence() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitCmdPipeSequence(this);
  }

  Cmd* cmd_left() const noexcept {
    return cmd_left_.get();
  }

  Cmd* cmd_right() const noexcept {
    return cmd_right_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Cmd> cmd_left_;
  std::unique_ptr<Cmd> cmd_right_;

  CmdPipeSequence(std::unique_ptr<Cmd> cmd_left,
                  std::unique_ptr<Cmd> cmd_right, Position position)
      : Cmd(NodeType::kCmdPipeSequence, position)
      , cmd_left_(std::move(cmd_left))
      , cmd_right_(std::move(cmd_right)) {}
};

class CmdPiece: public AstNode {
 public:
  ~CmdPiece() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitCmdPiece(this);
  }

  std::string cmd_str() {
    std::string str = Token::TokenValueToStr(token_.GetValue());

    return str;
  }

  bool blank_after() {
    return token_.BlankAfter();
  }

 private:
  friend class AstNodeFactory;

  Token token_;

  CmdPiece(const Token& token, Position position)
      : AstNode(NodeType::kCmdPiece, position)
      , token_(std::move(token)) {}
};

// class to support CmdIoRedirect
class FilePathCmd: public Cmd {
 public:
  ~FilePathCmd() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitFilePathCmd(this);
  }

  std::vector<AstNode*> children() noexcept {
    std::vector<AstNode*> vec;

    for (auto&& piece: pieces_) {
      vec.push_back(piece.get());
    }

    return vec;
  }

  size_t num_children() const noexcept {
    pieces_.size();
  }

 private:
  friend class AstNodeFactory;

  std::vector<std::unique_ptr<AstNode>> pieces_;

  FilePathCmd(std::vector<std::unique_ptr<AstNode>>&& pieces,
              Position position)
      : Cmd(NodeType::kFilePathCmd, position)
      , pieces_(std::move(pieces)) {}
};

class CmdIoRedirectList: public Cmd {
public:
 ~CmdIoRedirectList() {}

  virtual void Accept(AstVisitor* visitor) {
   visitor->VisitCmdIoRedirectList(this);
  }

  Cmd* cmd() const noexcept {
   return cmd_.get();
  }

  std::vector<CmdIoRedirect*> children() noexcept {
    std::vector<CmdIoRedirect*> vec;

    for (auto& io: io_list_) {
      vec.push_back(io.get());
    }

    return vec;
  }

  size_t num_children() const noexcept {
    io_list_.size();
  }

 private:
  friend class AstNodeFactory;

  std::vector<std::unique_ptr<CmdIoRedirect>> io_list_;
  std::unique_ptr<Cmd> cmd_;

  CmdIoRedirectList(std::unique_ptr<Cmd> cmd,
                    std::vector<std::unique_ptr<CmdIoRedirect>> io_list,
                    Position position)
     : Cmd(NodeType::kCmdIoRedirectList, position)
     , cmd_(std::move(cmd))
     , io_list_(std::move(io_list)) {}
};

class CmdIoRedirect: public Cmd {
 public:
  ~CmdIoRedirect() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitCmdIoRedirect(this);
  }

  TokenKind kind() const noexcept {
    return token_kind_;
  }

  bool has_integer() const noexcept {
    if (integer_) {
      return true;
    }

    return false;
  }

  //Verify is the output io interface is all interfaces
  // that is: "&>", means stdout and stderr
  bool all() const noexcept {
    return all_;
  }

  // IO interface number: 2> or 1>
  Literal* integer() const noexcept {
    return integer_.get();
  }

  FilePathCmd* file_path_cmd() const noexcept {
    return fp_cmd_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Literal> integer_;
  std::unique_ptr<FilePathCmd> fp_cmd_;
  TokenKind token_kind_;
  bool all_;


  CmdIoRedirect(std::unique_ptr<Literal> integer,
                std::unique_ptr<FilePathCmd> fp_cmd, TokenKind token_kind,
                bool all, Position position)
      : Cmd(NodeType::kCmdIoRedirect, position)
      , integer_(std::move(integer))
      , fp_cmd_(std::move(fp_cmd))
      , token_kind_(token_kind)
      , all_(all) {}
};

class SimpleCmd: public Cmd {
 public:
  ~SimpleCmd() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitSimpleCmd(this);
  }

  std::vector<AstNode*> children() noexcept {
    std::vector<AstNode*> vec;

    for (auto&& piece: pieces_) {
      vec.push_back(piece.get());
    }

    return vec;
  }

  size_t num_children() const noexcept {
    pieces_.size();
  }

 private:
  friend class AstNodeFactory;

  std::vector<std::unique_ptr<AstNode>> pieces_;

  SimpleCmd(std::vector<std::unique_ptr<AstNode>>&& pieces, Position position)
      : Cmd(NodeType::kSimpleCmd, position)
      , pieces_(std::move(pieces)) {}
};

class ForInStatement: public Statement {
 public:
  virtual ~ForInStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitForInStatement(this);
  }

  ExpressionList* exp_list() const noexcept {
    return exp_list_.get();
  }

  ExpressionList* test_list() const noexcept {
    return test_list_.get();
  }

  Statement* block() const noexcept {
    return block_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<ExpressionList> exp_list_;
  std::unique_ptr<ExpressionList> test_list_;
  std::unique_ptr<Statement> block_;

  ForInStatement(std::unique_ptr<ExpressionList> exp_list,
                 std::unique_ptr<ExpressionList> test_list,
                 std::unique_ptr<Statement> block,
                 Position position)
      : Statement(NodeType::kForInStatement, position)
      , exp_list_(std::move(exp_list))
      , test_list_(std::move(test_list))
      , block_(std::move(block)) {}
};

class SwitchStatement: public Statement {
 public:
  virtual ~SwitchStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitSwitchStatement(this);
  }

  Expression* exp() const noexcept {
    return exp_.get();
  }

  Statement* block() const noexcept {
    return block_.get();
  }

  bool has_exp() const noexcept {
    if (exp_) {
      return true;
    }

    return false;
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Expression> exp_;
  std::unique_ptr<Statement> block_;

  SwitchStatement(std::unique_ptr<Expression> exp,
                  std::unique_ptr<Statement> block,
                  Position position)
      : Statement(NodeType::kSwitchStatement, position)
      , exp_(std::move(exp))
      , block_(std::move(block)) {}
};

class CaseStatement: public Statement {
 public:
  virtual ~CaseStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitCaseStatement(this);
  }

  Expression* exp() const noexcept {
    return exp_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Expression> exp_;

  CaseStatement(std::unique_ptr<Expression> exp,
                Position position)
      : Statement(NodeType::kCaseStatement, position)
      , exp_(std::move(exp)) {}
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

class WhileStatement: public Statement {
 public:
  virtual ~WhileStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitWhileStatement(this);
  }

  Expression* exp() const noexcept {
    return exp_.get();
  }

  Statement* block() const noexcept {
    return block_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Expression> exp_;
  std::unique_ptr<Statement> block_;

  WhileStatement(std::unique_ptr<Expression> exp,
              std::unique_ptr<Statement> block,
              Position position)
      : Statement(NodeType::kIfStatement, position)
      , exp_(std::move(exp))
      , block_(std::move(block)) {}
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

  AssignableList* rvalue_list() const noexcept {
    return rvalue_.get();
  }

  bool has_rvalue() const noexcept {
    if (rvalue_) {
      return true;
    }

    return false;
  }

 private:
  friend class AstNodeFactory;

  TokenKind assign_kind_;
  std::unique_ptr<ExpressionList> lexp_;
  std::unique_ptr<AssignableList> rvalue_;

  AssignmentStatement(TokenKind assign_kind,
                      std::unique_ptr<ExpressionList> lexp,
                      std::unique_ptr<AssignableList> rvalue,
                      Position position)
      : Statement(NodeType::kAssignmentStatement, position)
      , assign_kind_(assign_kind)
      , lexp_(std::move(lexp))
      , rvalue_(std::move(rvalue)) {}
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

class BreakStatement: public Statement {
 public:
  virtual ~BreakStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitBreakStatement(this);
  }

 private:
  friend class AstNodeFactory;

  BreakStatement(Position position)
      : Statement(NodeType::kBreakStatement, position) {}
};

class DefaultStatement: public Statement {
 public:
  virtual ~DefaultStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitDefaultStatement(this);
  }

 private:
  friend class AstNodeFactory;

  DefaultStatement(Position position)
      : Statement(NodeType::kBreakStatement, position) {}
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

  PackageScope* scope() const noexcept {
    return scope_.get();
  }

  bool has_scope() const noexcept {
    if (scope_) {
      return true;
    }

    return false;
  }

 private:
  friend class AstNodeFactory;

  std::string name_;
  std::unique_ptr<PackageScope> scope_;

  Identifier(const std::string& name, std::unique_ptr<PackageScope> scope,
             Position position)
    : name_(name)
    , scope_(std::move(scope))
    , Expression(NodeType::kIdentifier, position) {}
};

class PackageScope: public AstNode {
 public:
  virtual ~PackageScope() {}

  virtual void Accept(AstVisitor* visitor) {
   visitor->VisitPackageScope(this);
  }

  Identifier* id() const noexcept {
   return id_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Identifier> id_;

  PackageScope(std::unique_ptr<Identifier> id, Position position):
   id_(std::move(id)), AstNode(NodeType::kPackageScope, position) {}
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

  inline std::unique_ptr<Identifier> NewIdentifier(
      const std::string& name, std::unique_ptr<PackageScope> scope =
          std::unique_ptr<PackageScope>(nullptr)) {
    return std::unique_ptr<Identifier>(new Identifier(
        name, std::move(scope), fn_pos_()));
  }

  inline std::unique_ptr<PackageScope> NewPackageScope(
      std::unique_ptr<Identifier> id) {
    return std::unique_ptr<PackageScope>(new PackageScope(
        std::move(id), fn_pos_()));
  }

  inline std::unique_ptr<AssignmentStatement> NewAssignmentStatement(
      TokenKind assign_kind, std::unique_ptr<ExpressionList> lexp_list,
      std::unique_ptr<AssignableList> rvalue_list) {
    return std::unique_ptr<AssignmentStatement>(new AssignmentStatement(
        assign_kind, std::move(lexp_list), std::move(rvalue_list), fn_pos_()));
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

  inline std::unique_ptr<BreakStatement> NewBreakStatement() {
    return std::unique_ptr<BreakStatement>(new BreakStatement(fn_pos_()));
  }

  inline std::unique_ptr<DefaultStatement> NewDefaultStatement() {
    return std::unique_ptr<DefaultStatement>(new DefaultStatement(fn_pos_()));
  }

  inline std::unique_ptr<IfStatement> NewIfStatement(
      std::unique_ptr<Expression> exp,
      std::unique_ptr<Statement> then_block,
      std::unique_ptr<Statement> else_block) {
    return std::unique_ptr<IfStatement>(new IfStatement(
        std::move(exp), std::move(then_block), std::move(else_block),
        fn_pos_()));
  }

  inline std::unique_ptr<WhileStatement> NewWhileStatement(
      std::unique_ptr<Expression> exp,
      std::unique_ptr<Statement> block) {
    return std::unique_ptr<WhileStatement>(new WhileStatement(
        std::move(exp), std::move(block), fn_pos_()));
  }

  inline std::unique_ptr<SwitchStatement> NewSwitchStatement(
      std::unique_ptr<Expression> exp,
      std::unique_ptr<Statement> block) {
    return std::unique_ptr<SwitchStatement>(new SwitchStatement(
        std::move(exp), std::move(block), fn_pos_()));
  }

  inline std::unique_ptr<ForInStatement> NewForInStatement(
      std::unique_ptr<ExpressionList> exp_list,
      std::unique_ptr<ExpressionList> test_list,
      std::unique_ptr<Statement> block) {
    return std::unique_ptr<ForInStatement>(new ForInStatement(
        std::move(exp_list), std::move(test_list), std::move(block),
        fn_pos_()));
  }

  inline std::unique_ptr<CaseStatement> NewCaseStatement(
      std::unique_ptr<Expression> exp) {
    return std::unique_ptr<CaseStatement>(new CaseStatement(
        std::move(exp), fn_pos_()));
  }

  inline std::unique_ptr<CmdPiece> NewCmdPiece(const Token& token) {
    return std::unique_ptr<CmdPiece>(new CmdPiece(token, fn_pos_()));
  }

  inline std::unique_ptr<SimpleCmd> NewSimpleCmd(
      std::vector<std::unique_ptr<AstNode>>&& pieces) {
    return std::unique_ptr<SimpleCmd>(new SimpleCmd(
        std::move(pieces), fn_pos_()));
  }

  inline std::unique_ptr<FilePathCmd> NewFilePathCmd(
      std::vector<std::unique_ptr<AstNode>>&& pieces) {
    return std::unique_ptr<FilePathCmd>(new FilePathCmd(
        std::move(pieces), fn_pos_()));
  }

  inline std::unique_ptr<CmdIoRedirect> NewCmdIoRedirect(
      std::unique_ptr<Literal> integer, std::unique_ptr<FilePathCmd> fp_cmd,
      TokenKind kind, bool all) {
    return std::unique_ptr<CmdIoRedirect>(new CmdIoRedirect(
        std::move(integer), std::move(fp_cmd), kind, all, fn_pos_()));
  }

  inline std::unique_ptr<CmdIoRedirectList> NewCmdIoRedirectList(
      std::unique_ptr<Cmd> cmd,
      std::vector<std::unique_ptr<CmdIoRedirect>>&& io_list) {
    return std::unique_ptr<CmdIoRedirectList>(new CmdIoRedirectList(
        std::move(cmd), std::move(io_list), fn_pos_()));
  }

  inline std::unique_ptr<CmdPipeSequence> NewCmdPipeSequence(
      std::unique_ptr<Cmd> cmd_left, std::unique_ptr<Cmd> cmd_right) {
    return std::unique_ptr<CmdPipeSequence>(new CmdPipeSequence(
        std::move(cmd_left),  std::move(cmd_right), fn_pos_()));
  }

  inline std::unique_ptr<CmdAndOr> NewCmdAndOr(
      TokenKind token_kind, std::unique_ptr<Cmd> cmd_left,
      std::unique_ptr<Cmd> cmd_right) {
    return std::unique_ptr<CmdAndOr>(new CmdAndOr(
        token_kind, std::move(cmd_left),  std::move(cmd_right), fn_pos_()));
  }

  inline std::unique_ptr<CmdFull> NewCmdFull(std::unique_ptr<Cmd> cmd,
                                             bool background) {
    return std::unique_ptr<CmdFull>(new CmdFull(
        std::move(cmd),  background, fn_pos_()));
  }

  inline std::unique_ptr<CmdExpression> NewCmdExpression(
      std::unique_ptr<Cmd> cmd) {
    return std::unique_ptr<CmdExpression>(new CmdExpression(
        std::move(cmd), fn_pos_()));
  }

  inline std::unique_ptr<FunctionParam> NewFunctionParam(
      std::unique_ptr<Identifier> id, bool variadic) {
    return std::unique_ptr<FunctionParam>(new FunctionParam(
        std::move(id), variadic, fn_pos_()));
  }

  inline std::unique_ptr<FunctionDeclaration> NewFunctionDeclaration(
      std::vector<std::unique_ptr<FunctionParam>>&& params,
      std::unique_ptr<Identifier> name,
      std::unique_ptr<Block> block) {
    return std::unique_ptr<FunctionDeclaration>(new FunctionDeclaration(
        std::move(params), std::move(name), std::move(block), fn_pos_()));
  }

  inline std::unique_ptr<ArrayInstantiation> NewArrayInstantiation(
      std::unique_ptr<AssignableList> elements) {
    return std::unique_ptr<ArrayInstantiation>(new ArrayInstantiation(
        std::move(elements), fn_pos_()));
  }

  template<class T>
  inline std::unique_ptr<AssignableValue> NewAssignableValue(
      std::unique_ptr<T>&& value) {
    return std::unique_ptr<AssignableValue>(new AssignableValue(
        std::move(value), fn_pos_()));
  }

  inline std::unique_ptr<AssignableList> NewAssignableList(
      std::vector<std::unique_ptr<AssignableValue>>&& nodes) {
    return std::unique_ptr<AssignableList>(new AssignableList(
        std::move(nodes), fn_pos_()));
  }

  inline std::unique_ptr<KeyValue> NewKeyValue(
      std::unique_ptr<Expression> key, std::unique_ptr<AssignableValue> value) {
    return std::unique_ptr<KeyValue>(new KeyValue(
        std::move(key), std::move(value), fn_pos_()));
  }

  inline std::unique_ptr<DictionaryInstantiation> NewDictionaryInstantiation(
      std::vector<std::unique_ptr<KeyValue>>&& key_value_list) {
    return std::unique_ptr<DictionaryInstantiation>(new DictionaryInstantiation(
        std::move(key_value_list), fn_pos_()));
  }

  inline std::unique_ptr<ReturnStatement> NewReturnStatement(
      std::unique_ptr<AssignableList> assign_list) {
    return std::unique_ptr<ReturnStatement>(new ReturnStatement(
        std::move(assign_list), fn_pos_()));
  }

  inline std::unique_ptr<CmdDeclaration> NewCmdDeclaration(
      std::unique_ptr<Identifier> id, std::unique_ptr<Block> block) {
    return std::unique_ptr<CmdDeclaration>(new CmdDeclaration(
        std::move(id), std::move(block), fn_pos_()));
  }

  inline std::unique_ptr<SubShell> NewSubShell(std::unique_ptr<Block> block) {
    return std::unique_ptr<SubShell>(new SubShell(std::move(block), fn_pos_()));
  }

 private:
  std::function<Position()> fn_pos_;
};

}
}

#endif  // SETTI_AST_H


