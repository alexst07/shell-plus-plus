#ifndef SETI_EXPR_EXECUTOR_H
#define SETI_EXPR_EXECUTOR_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <tuple>

#include "ast/ast.h"
#include "ast/obj_type.h"
#include "executor.h"

namespace setti {
namespace internal {

class AssignableListExecutor: public Executor {
 public:
  AssignableListExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  std::vector<ObjectPtr> Exec(AstNode* node);

  ObjectPtr ExecAssignable(AstNode* node);
};

class ExpressionExecutor: public Executor {
 public:
  ExpressionExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

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

  // Pass the variable as value or reference depending on type
  inline ObjectPtr PassVar(ObjectPtr obj) {
    switch (obj->type()) {
      case Object::ObjectType::NIL:
        return ObjectPtr(new NullObject());
        break;

      case Object::ObjectType::INT:
        return ObjectPtr(new IntObject(static_cast<IntObject&>(*obj)));
        break;

      case Object::ObjectType::BOOL:
        return ObjectPtr(new BoolObject(static_cast<BoolObject&>(*obj)));
        break;

      case Object::ObjectType::REAL:
        return ObjectPtr(new RealObject(static_cast<RealObject&>(*obj)));
        break;

      case Object::ObjectType::STRING:
        return ObjectPtr(new StringObject(static_cast<StringObject&>(*obj)));
        break;

      default:
        return obj;
    }
  }

  // Executes array access, it could be a language array, map, tuple or
  // custon object
  ObjectPtr ExecArrayAccess(AstNode* node);

  // Access a position of array object
  ObjectPtr ArrayAccess(Array& array_node, ArrayObject& obj);

  // Access a position of tuple object
  ObjectPtr TupleAccess(Array& array_node, TupleObject& obj);

  // Executes array instantiation
  ObjectPtr ExecArrayInstantiation(AstNode* node);

  // Executes map instantiation
  ObjectPtr ExecMapInstantiation(AstNode* node);

};

}
}

#endif  // SETI_EXPR_EXECUTOR_H


