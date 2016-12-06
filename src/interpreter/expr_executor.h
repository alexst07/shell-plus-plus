#ifndef SETI_EXPR_EXECUTOR_H
#define SETI_EXPR_EXECUTOR_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <tuple>

#include "ast/ast.h"
#include "obj_type.h"
#include "executor.h"
#include "object-factory.h"

namespace setti {
namespace internal {

class AssignableListExecutor: public Executor {
 public:
  AssignableListExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  std::vector<ObjectPtr> Exec(AstNode* node);

  ObjectPtr ExecAssignable(AstNode* node);

  void set_stop(StopFlag flag) override;
};

class ExpressionExecutor: public Executor {
 public:
  ExpressionExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack)
      , obj_factory(symbol_table_stack) {}

  // Entry point to execute expression
  ObjectPtr Exec(AstNode* node);

  // Executes literal const and return an object with its value
  ObjectPtr ExecLiteral(AstNode* node);

  // Lookup on symbol table and return a reference
  // if the object is simple as integer, bool, string or real
  // then create a copy the object and return its reference
  // if it is a container as array, tuple, or map
  // return only the reference of the object on symbol table
  ObjectPtr ExecIdentifier(AstNode* node);

  // Executes array access, it could be a language array, map, tuple or
  // custon object
  ObjectPtr ExecArrayAccess(AstNode* node);

  // Access a position of array object
  ObjectPtr ArrayAccess(Array& array_node, ArrayObject& obj);

  // Access a position of tuple object
  ObjectPtr TupleAccess(Array& array_node, TupleObject& obj);

  // Access a position of tuple object
  ObjectPtr MapAccess(Array& array_node, MapObject& obj);

  // Executes array instantiation
  ObjectPtr ExecArrayInstantiation(AstNode* node);

  // Executes map instantiation
  ObjectPtr ExecMapInstantiation(AstNode* node);

  // Executes function call
  ObjectPtr ExecFuncCall(FunctionCall* node);

  // Executes binary operation
  ObjectPtr ExecBinOp(BinaryOperation* node);

  // Executes attribute
  ObjectPtr ExecAttribute(Attribute* node);

  void set_stop(StopFlag flag) override;

 private:
  ObjectFactory obj_factory;
};

class FuncCallExecutor: public Executor {
 public:
  FuncCallExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  // Entry point to execute expression
  ObjectPtr Exec(FunctionCall* node);

  void set_stop(StopFlag flag) override;
};

// Pass the variable as value or reference depending on type
inline ObjectPtr PassVar(ObjectPtr obj, SymbolTableStack& symbol_table_stack) {
  ObjectFactory obj_factory(symbol_table_stack);
  switch (obj->type()) {
    case Object::ObjectType::NIL:
      return obj_factory.NewNull();
      break;

    case Object::ObjectType::INT:
      return obj_factory.NewInt(static_cast<IntObject&>(*obj).value());
      break;

    case Object::ObjectType::BOOL:
      return obj_factory.NewBool(static_cast<BoolObject&>(*obj).value());
      break;

    case Object::ObjectType::REAL:
      return obj_factory.NewReal(static_cast<RealObject&>(*obj).value());
      break;

    case Object::ObjectType::STRING:
      return obj_factory.NewString(static_cast<StringObject&>(*obj).value());
      break;

    default:
      return obj;
  }
}

}
}

#endif  // SETI_EXPR_EXECUTOR_H


