#include "obj_type.h"

#include <string>
#include <boost/variant.hpp>

#include "str-object.h"
#include "array-object.h"
#include "object-factory.h"
#include "stmt_executor.h"

namespace setti {
namespace internal {

MapObject::MapObject(std::vector<std::pair<ObjectPtr, ObjectPtr>>&& value,
                     ObjectPtr obj_type, SymbolTableStack&& sym_table)
    : Object(ObjectType::MAP, obj_type, std::move(sym_table)) {
  for (auto& e: value) {
    std::vector<std::pair<ObjectPtr, ObjectPtr>> list;
    list.push_back(e);
    value_.insert(std::pair<size_t, std::vector<std::pair<ObjectPtr,
        ObjectPtr>>>(e.first->Hash(), list));
  }
}

bool MapObject::operator==(const Object& obj) const {
  if (obj.type() != ObjectType::MAP) {
    return false;
  }

  using ls = std::vector<std::pair<ObjectPtr, ObjectPtr>>;
  const MapObject& map = static_cast<const MapObject&>(obj);

  // for to compare two maps
  for (struct {Map::const_iterator a; Map::const_iterator b;} loop
           = { value_.begin(), map.value_.begin() };
       (loop.a != value_.end()) && (loop.b != map.value_.end());
       loop.a++, loop.b++) {
    // for to compare the lists inside the maps
    for (struct {ls::const_iterator la; ls::const_iterator lb;} l
             = { loop.a->second.begin(), loop.b->second.begin() };
         (l.la != loop.a->second.end()) && (l.lb != loop.b->second.end());
         l.la++, l.lb++) {
      if (*l.la != *l.lb) {
        return false;
      }
    }
  }

  return true;
}

std::shared_ptr<Object> MapObject::Element(ObjectPtr obj_index) {
  size_t hash = obj_index->Hash();

  auto it = value_.find(hash);

  // return a tuple with null object and false bool object
  auto error = []() {
    throw RunTimeError(RunTimeError::ErrorCode::OUT_OF_RANGE,
                       boost::format("key not found"));
  };

  // if the index not exists on the map return a tuple object
  // with null and bool object
  if (it == value_.end()) {
    error();
  }

  // if the index exists on map, search the object on the list, to confirm
  // that is not a false hash match
  for (auto& e: it->second) {
    // when the obj_index match with any index on the list, return this item
    if (*e.first == *obj_index) {
      return e.second;
    } else {
      error();
    }
  }
}

ObjectPtr& MapObject::Insert_(ObjectPtr obj_index) {
  size_t hash = obj_index->Hash();

  auto it = value_.find(hash);
  ObjectPtr obj(nullptr);

  // if the hash doesn't exists create a entry with a list
  if (it == value_.end()) {
    std::vector<std::pair<ObjectPtr, ObjectPtr>> list;
    list.push_back(std::pair<ObjectPtr, ObjectPtr>(obj_index, obj));
    value_.insert(Pair(hash, list));
  } else {
    it->second.push_back(std::pair<ObjectPtr, ObjectPtr>(obj_index, obj));
  }

  return value_.find(hash)->second.back().second;
}

bool MapObject::Exists(ObjectPtr obj_index) {
  size_t hash = obj_index->Hash();

  auto it = value_.find(hash);

  if (it != value_.end()) {
    for (auto& e: it->second) {
      if (*e.first == *obj_index) {
        return true;
      }
    }
  }

  return false;
}

ObjectPtr TypeObject::CallObject(const std::string& name,
                                 ObjectPtr self_param) {
  ObjectPtr obj = sym_tab_statck_->Lookup(name, false).SharedAccess();

  if (obj->type() == ObjectType::FUNC) {
    ObjectFactory obj_factory(*sym_tab_statck_);

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

std::shared_ptr<Object> DeclClassType::Arrow(std::shared_ptr<Object> self,
                              const std::string& name) {
  ObjectPtr att_obj = symbol_table_stack().Lookup(name, false).SharedAccess();

  return att_obj;
}

std::shared_ptr<Object> DeclClassObject::Arrow(std::shared_ptr<Object> self,
                              const std::string& name) {
  SymbolTableStack& st =
      static_cast<DeclClassType&>(*ObjType()).SymTableStack();
  ObjectPtr att_obj = st.Lookup(name, false).SharedAccess();

  if (att_obj->type() == ObjectType::FUNC) {
    return static_cast<DeclClassType&>(*ObjType()).CallObject(name, self);
  }

  return att_obj;
}

std::shared_ptr<Object>& DeclClassObject::ArrowAssign(
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

  switch (params[0]->type()) {
    case ObjectType::INT: {
      ObjectFactory obj_factory(symbol_table_stack());
      ObjectPtr obj_int(obj_factory.NewInt(static_cast<IntObject&>(
          *params[0]).value()));

      return obj_int;
    } break;

    case ObjectType::STRING: {
      const StringObject& str_obj =
          static_cast<const StringObject&>(*params[0]);
      int v = Type2Int(str_obj.value());

      ObjectFactory obj_factory(symbol_table_stack());
      ObjectPtr obj_int(obj_factory.NewInt(v));
      return obj_int;
    } break;

    case ObjectType::REAL: {
      const RealObject& real_obj =
          static_cast<const RealObject&>(*params[0]);
      int v = Type2Int(real_obj.value());

      ObjectFactory obj_factory(symbol_table_stack());
      ObjectPtr obj_int(obj_factory.NewReal(v));
      return obj_int;
    } break;

    default:
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("invalid conversion to int"));
  }
}

ObjectPtr RealType::Constructor(Executor* /*parent*/,
                                std::vector<ObjectPtr>&& params) {
  if (params.size() != 1) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                       boost::format("real() takes exactly 1 argument"));
  }

  switch (params[0]->type()) {
    case ObjectType::REAL: {
      ObjectFactory obj_factory(symbol_table_stack());

      ObjectPtr obj_real(obj_factory.NewReal(static_cast<RealObject&>(
          *params[0]).value()));

      return obj_real;
    } break;

    case ObjectType::STRING: {
      const StringObject& str_obj =
          static_cast<const StringObject&>(*params[0]);
      int v = Type2Real(str_obj.value());

      ObjectFactory obj_factory(symbol_table_stack());
      ObjectPtr obj_real(obj_factory.NewReal(v));

      return obj_real;
    } break;

    case ObjectType::INT: {
      const IntObject& int_obj =
          static_cast<const IntObject&>(*params[0]);
      float v = Type2Real(int_obj.value());

      ObjectFactory obj_factory(symbol_table_stack());
      ObjectPtr obj_real(obj_factory.NewReal(v));

      return obj_real;
    } break;

    default:
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("invalid conversion to real"));
  }
}

ObjectPtr StringType::Constructor(Executor* /*parent*/,
                                  std::vector<ObjectPtr>&& params) {
  if (params.size() != 1) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                       boost::format("real() takes exactly 1 argument"));
  }

  switch (params[0]->type()) {
    case ObjectType::STRING: {
      ObjectFactory obj_factory(symbol_table_stack());

      ObjectPtr obj_str(obj_factory.NewString(static_cast<StringObject&>(
          *params[0]).value()));

      return obj_str;
    } break;

    case ObjectType::REAL: {
      const RealObject& obj_real =
          static_cast<const RealObject&>(*params[0]);

      std::string v = std::to_string(obj_real.value());

      ObjectFactory obj_factory(symbol_table_stack());
      ObjectPtr obj_str(obj_factory.NewString(v));

      return obj_str;
    } break;

    case ObjectType::INT: {
      const IntObject& int_obj =
          static_cast<const IntObject&>(*params[0]);

      std::string v = std::to_string(int_obj.value());

      ObjectFactory obj_factory(symbol_table_stack());
      ObjectPtr obj_str(obj_factory.NewString(v));

      return obj_str;
    } break;

    default:
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("invalid conversion to string"));
  }
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

}
}
