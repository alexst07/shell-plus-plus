#include "obj_type.h"

#include <string>
#include <boost/variant.hpp>

#include "object-factory.h"
#include "stmt_executor.h"

namespace setti {
namespace internal {

std::size_t ArrayObject::Hash() const {
  if (value_.empty()) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("hash of empty array is not valid"));
  }

  size_t hash = 0;

  // Executes xor operation with hash of each element of array
  for (auto& e: value_) {
    hash ^= e->Hash();
  }

  return hash;
}

bool ArrayObject::operator==(const Object& obj) const {
  if (obj.type() != ObjectType::ARRAY) {
    return false;
  }

  const ArrayObject& array_obj = static_cast<const ArrayObject&>(obj);

  // If the tuples have different size, they are different
  if (array_obj.value_.size() != value_.size()) {
    return false;
  }

  bool r = true;

  // Test each element on tuple
  for (size_t i = 0; i < value_.size(); i++) {
    r = r && (array_obj.value_[i] == value_[i]);
  }

  return r;
}

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

ObjectPtr FuncDeclObject::Call(Executor* parent,
                               std::vector<ObjectPtr>&& params) {
  if (variadic_) {
    if (params.size() < (params_.size() - 1)) {
      throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
          boost::format("%1% takes at least %2% argument (%3% given)")%
                        id_% (params_.size() - 1)% params.size());
    }

    // Insert objects on symbol table
    for (size_t i = 0; i < params_.size() - 1; i++) {
      symbol_table_.SetEntry(params_[i], params[i]);
    }

    // The others given parameters is transformed in a tuple
    std::vector<ObjectPtr> vec_params;

    for (size_t i = params_.size() - 1; i < params.size(); i++) {
      vec_params.push_back(params[i]);
    }

    ObjectFactory obj_factory(symbol_table_stack());
    ObjectPtr tuple_obj(obj_factory.NewTuple(std::move(vec_params)));

    symbol_table_.SetEntry(params_[params_.size() - 1], tuple_obj);
  } else {
    if ((params.size() < (params_.size() - default_values_.size())) ||
        (params.size() > params_.size())) {
      throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
          boost::format("%1% takes exactly %2% argument (%3% given)")%
                        id_% (params_.size() - 1)% params.size());
    }

    // Insert objects on symbol table
    for (size_t i = 0; i < params.size(); i++) {
      symbol_table_.SetEntry(params_[i], params[i]);
    }

    size_t no_values_params = params_.size() - default_values_.size();

    for (size_t i = no_values_params; i < params_.size(); i++) {
      symbol_table_.SetEntry(params_[i],
                             default_values_[i - no_values_params]);
    }
  }

  // Executes the function using the ast nodes
  BlockExecutor executor(parent, symbol_table_);
  executor.Exec(start_node_);

  ObjectPtr obj_ret;
  bool bool_ret = false;
  std::tie(obj_ret, bool_ret) = symbol_table_.LookupObj("%return");

  if (bool_ret) {
    return obj_ret;
  } else {
    ObjectFactory obj_factory(symbol_table_stack());
    return ObjectPtr(obj_factory.NewNull());
  }
}

ObjectPtr Type::Constructor(Executor* /*parent*/,
                            std::vector<ObjectPtr>&& params) {
  if (params.size() != 1) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                       boost::format("type() takes exactly 1 argument"));
  }

  ObjectPtr obj_type = params[0]->ObjType();
  if (!obj_type) {
    ObjectFactory obj_factory(symbol_table_stack());
    return ObjectPtr(obj_factory.NewType());
  } else {
    return obj_type;
  }
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

  bool b = params[0]->ObjBool();

  ObjectFactory obj_factory(symbol_table_stack());
  ObjectPtr bool_obj(obj_factory.NewBool(b));

  return bool_obj;
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


}
}
