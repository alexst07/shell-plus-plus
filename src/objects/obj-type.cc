#include "obj-type.h"

#include <string>
#include <boost/variant.hpp>

#include "str-object.h"
#include "array-object.h"
#include "object-factory.h"
#include "interpreter/stmt-executor.h"

namespace setti {
namespace internal {

ObjectPtr TypeObject::CallObject(const std::string& name,
                                 ObjectPtr self_param) {
  ObjectPtr obj = symbol_table_stack().Lookup(name, false).SharedAccess();

  if (obj->type() == ObjectType::FUNC) {
    ObjectFactory obj_factory(symbol_table_stack());

    // the function wrapper insert the object self_param as the first param
    // it works like self argument
    return ObjectPtr(obj_factory.NewWrapperFunc(obj, self_param));
  }

  return obj;
}

ObjectPtr Type::Constructor(Executor* /*parent*/,
                            std::vector<ObjectPtr>&& params) {
  if (params.size() != 1) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                       boost::format("type() takes exactly 1 argument"));
  }

  // get the type of object passed
  ObjectPtr obj_type = params[0]->ObjType();

  // if the object is null so it is a type, because when created
  // type has no type, because it will be itself
  if (!obj_type) {
    ObjectFactory obj_factory(symbol_table_stack());
    return ObjectPtr(obj_factory.NewType());
  } else {
    return obj_type;
  }
}

// constructor for declared class call __init__ method from
// symbol table, and create an DeclClassObject, this object
// has a symbol table stack to store attributes
ObjectPtr DeclClassType::Constructor(Executor* parent,
                                std::vector<ObjectPtr>&& params) {
  std::cout << ">>>> " << this->name() << "\n";
  symbol_table_stack().Dump();

  ObjectFactory obj_factory(symbol_table_stack());
  ObjectPtr obj_self(obj_factory.NewDeclObject(this->name()));

  ObjectPtr obj_init = symbol_table_stack().Lookup("__init__", false)
      .SharedAccess();

  if (obj_init->type() == ObjectType::FUNC) {
    params.insert(params.begin(), obj_self);
    static_cast<FuncObject&>(*obj_init).Call(parent, std::move(params));
  }

  return obj_self;
}

ObjectPtr DeclClassType::CallObject(const std::string& name,
                                 ObjectPtr self_param) {
  ObjectPtr obj = symbol_table_stack().Lookup(name, false).SharedAccess();

  if (obj->type() == ObjectType::FUNC) {
    ObjectFactory obj_factory(symbol_table_stack());

    // the function wrapper insert the object self_param as the first param
    // it works like self argument
    return ObjectPtr(obj_factory.NewWrapperFunc(obj, self_param));
  }

  return obj;
}

std::shared_ptr<Object> DeclClassType::Attr(std::shared_ptr<Object> self,
                              const std::string& name) {
  ObjectPtr att_obj = symbol_table_stack().Lookup(name, false).SharedAccess();

  return att_obj;
}

std::shared_ptr<Object> DeclClassObject::Attr(std::shared_ptr<Object> self,
                              const std::string& name) {
  SymbolTableStack& st =
      static_cast<DeclClassType&>(*ObjType()).SymTableStack();
  ObjectPtr att_obj = st.Lookup(name, false).SharedAccess();

  if (att_obj->type() == ObjectType::FUNC) {
    return static_cast<DeclClassType&>(*ObjType()).CallObject(name, self);
  }

  return att_obj;
}

std::shared_ptr<Object>& DeclClassObject::AttrAssign(
    std::shared_ptr<Object> /*self*/, const std::string& name) {
  SymbolTableStack& st =
      static_cast<DeclClassType&>(*ObjType()).SymTableStack();
  ObjectPtr& att_obj = st.Lookup(name, true).Ref();

  return att_obj;
}

// TODO: insert some protection avoid __add__ method with more than 1
// parameter or none parameter
ObjectPtr DeclClassObject::Add(ObjectPtr obj) {
  SymbolTableStack& st =
      static_cast<DeclClassType&>(*ObjType()).SymTableStack();
  ObjectPtr func_obj = st.Lookup("__add__", false).SharedAccess();

  if (func_obj->type() != ObjectType::FUNC) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("symbol __add__ must be func"));
  }

  // insert self object on parameters list
  std::vector<ObjectPtr> params;
  params.push_back(self_.lock());
  params.push_back(obj);

  return static_cast<FuncObject&>(*func_obj).Call(nullptr, std::move(params));
}

std::shared_ptr<Object> ModuleImportObject::Attr(std::shared_ptr<Object>/*self*/,
                                           const std::string& name) {
  auto obj = SymTableStack().Lookup(name, false).Ref();
  return PassVar(obj, symbol_table_stack());
}

std::shared_ptr<Object> ModuleCustonObject::Attr(std::shared_ptr<Object>/*self*/,
                                           const std::string& name) {
  // search on symbol table of the module
  auto obj = symbol_table_stack_.Lookup(name, false).Ref();

  // PassVar uses the global symbol table because it uses types as int ans real
  return PassVar(obj, symbol_table_stack());
}

ObjectPtr NullType::Constructor(Executor* /*parent*/,
                                std::vector<ObjectPtr>&& params) {
  if (params.size() > 0) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                       boost::format("null_t() takes no arguments"));
  }

  ObjectFactory obj_factory(symbol_table_stack());
  return ObjectPtr(obj_factory.NewNull());
}

ObjectPtr BoolType::Constructor(Executor* /*parent*/,
                                std::vector<ObjectPtr>&& params) {
  if (params.size() != 1) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                       boost::format("bool() takes exactly 1 argument"));
  }

  return params[0]->ObjBool();
}

ObjectPtr IntType::Constructor(Executor* /*parent*/,
                               std::vector<ObjectPtr>&& params) {
  if (params.size() != 1) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                       boost::format("int() takes exactly 1 argument"));
  }

  if (params[0]->type() == ObjectType::INT) {
    ObjectFactory obj_factory(symbol_table_stack());
    ObjectPtr obj_int(obj_factory.NewInt(static_cast<IntObject&>(
        *params[0]).value()));

    return obj_int;
  }

  return params[0]->ObjInt();
}

ObjectPtr RealType::Constructor(Executor* /*parent*/,
                                std::vector<ObjectPtr>&& params) {
  if (params.size() != 1) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                       boost::format("real() takes exactly 1 argument"));
  }

  if (params[0]->type() == ObjectType::REAL) {
    ObjectFactory obj_factory(symbol_table_stack());

    ObjectPtr obj_real(obj_factory.NewReal(static_cast<RealObject&>(
        *params[0]).value()));

    return obj_real;
  }

  params[0]->ObjReal();
}

ObjectPtr ArrayIterType::Constructor(Executor* /*parent*/,
                                     std::vector<ObjectPtr>&& params) {
  if (params.size() != 1) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                       boost::format("array_iter() takes exactly 1 argument"));
  }

  if (params[0]->type() != ObjectType::ARRAY) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("invalid type for array_iter"));
  }

  ObjectFactory obj_factory(symbol_table_stack());
  ObjectPtr obj(obj_factory.NewArrayIter(params[0]));
  return obj;
}

ObjectPtr CmdType::Constructor(Executor* /*parent*/,
                               std::vector<ObjectPtr>&& /*params*/) {
  throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                     boost::format("cmdobj is not constructable"));
}

ObjectPtr CmdIterType::Constructor(Executor* /*parent*/,
                                   std::vector<ObjectPtr>&& /*params*/) {
  throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                     boost::format("cmd_iter is not constructable"));
}

ObjectPtr ModuleType::Constructor(Executor* /*parent*/,
                                  std::vector<ObjectPtr>&& params) {
  throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                     boost::format("module is not constructable"));
}

}
}
