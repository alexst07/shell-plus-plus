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

#include "obj-type.h"

#include <boost/variant.hpp>
#include <string>

#include "array-object.h"
#include "interpreter/stmt-executor.h"
#include "object-factory.h"
#include "str-object.h"
#include "utils/check.h"

namespace shpp {
namespace internal {

RangeIterObject::RangeIterObject(int start, int end, int step,
                                 ObjectPtr obj_type,
                                 SymbolTableStack&& sym_table)
    : BaseIter(ObjectType::ARRAY_ITER, obj_type, std::move(sym_table)),
      start_(start),
      step_(step),
      end_(end),
      value_(start_) {}

ObjectPtr RangeIterObject::Next() {
  ObjectFactory obj_factory(symbol_table_stack());
  ObjectPtr ret_obj = obj_factory.NewInt(value_);

  value_ += step_;
  return ret_obj;
}

ObjectPtr RangeIterObject::HasNext() {
  ObjectFactory obj_factory(symbol_table_stack());

  if (step_ > 0) {
    if (value_ >= end_) {
      return obj_factory.NewBool(false);
    }

    return obj_factory.NewBool(true);
  }

  if (value_ <= end_) {
    return obj_factory.NewBool(false);
  }

  return obj_factory.NewBool(true);
}

ObjectPtr RangeIterObject::Equal(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());

  if (obj->type() != ObjectType::RANGE_ITER) {
    return obj_factory.NewBool(false);
  }

  RangeIterObject& range_it = static_cast<RangeIterObject&>(*obj);

  bool v = start_ == range_it.start_ && step_ == range_it.step_ &&
           end_ == range_it.end_ && value_ == range_it.value_;

  return obj_factory.NewBool(v);
}

ObjectPtr TypeObject::CallObject(const std::string& name,
                                 ObjectPtr self_param) {
  ObjectPtr obj = sym_table_->Lookup(name, false).SharedAccess();

  if (obj->type() == ObjectType::FUNC) {
    ObjectFactory obj_factory(symbol_table_stack());

    // the function wrapper insert the object self_param as the first param
    // it works like self argument
    return ObjectPtr(obj_factory.NewWrapperFunc(obj, self_param));
  }

  return obj;
}

ObjectPtr TypeObject::CallStaticObject(const std::string& name) {
  ObjectPtr obj = sym_table_->Lookup(name, false).SharedAccess();
  return obj;
}

ObjectPtr TypeObject::Equal(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());

  bool v = this->operator==(*obj);
  return obj_factory.NewBool(v);
}

ObjectPtr TypeObject::SearchAttr(const std::string& name) {
  auto it = sym_table_->Lookup(name);
  if (it != sym_table_->end()) {
    auto obj_ref = it->second.Ref();
    return PassVar(obj_ref, symbol_table_stack());
  }

  ObjectPtr base = BaseType();

  if (!base) {
    // if there are no base super class, so the attribute was not found
    throw RunTimeError(RunTimeError::ErrorCode::SYMBOL_NOT_FOUND,
                       boost::format("symbol %1% not found") % name);
  }

  if (base->type() != Object::ObjectType::TYPE) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("'%1%' is not a valid type for super"
                                     " class") %
                           base->ObjectName());
  }

  auto obj_ref = static_cast<TypeObject&>(*base).SearchAttr(name);
  return PassVar(obj_ref, symbol_table_stack());
}

ObjectPtr& TypeObject::SearchAttrRef(const std::string& name) {
  auto it = sym_table_->Lookup(name);
  if (it != sym_table_->end()) {
    return it->second.Ref();
  }

  ObjectPtr base = BaseType();

  if (!base) {
    // if there are no base super class, so the attribute was not found
    throw RunTimeError(RunTimeError::ErrorCode::SYMBOL_NOT_FOUND,
                       boost::format("symbol %1% not found") % name);
  }

  if (base->type() != Object::ObjectType::TYPE) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("'%1%' is not a valid type for super"
                                     " class") %
                           base->ObjectName());
  }

  return static_cast<TypeObject&>(*base).SearchAttrRef(name);
}

bool TypeObject::ExistsAttr(const std::string& name) {
  if (symbol_table_stack().Exists(name)) {
    return true;
  }

  ObjectPtr base = BaseType();

  if (!base) {
    return false;
  }

  if (base->type() != Object::ObjectType::TYPE) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("'%1%' is not a valid type for super"
                                     " class") %
                           base->ObjectName());
  }

  return static_cast<TypeObject&>(*base).ExistsAttr(name);
}

ObjectPtr Type::Constructor(Executor*, Args&& params, KWArgs&&) {
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

std::shared_ptr<Object> ModuleImportObject::Attr(
    std::shared_ptr<Object> /*self*/, const std::string& name) {
  auto obj = SymTableStack().Lookup(name, false).Ref();
  return PassVar(obj, symbol_table_stack());
}

std::shared_ptr<Object> ModuleMainObject::Attr(std::shared_ptr<Object> /*self*/,
                                               const std::string& name) {
  auto obj = SymTableStack().Lookup(name, false).Ref();
  return PassVar(obj, symbol_table_stack());
}

std::shared_ptr<Object> ModuleCustonObject::Attr(
    std::shared_ptr<Object> /*self*/, const std::string& name) {
  // search on symbol table of the module
  auto obj = symbol_table_stack_.Lookup(name, false).Ref();

  // PassVar uses the global symbol table because it uses types as int ans real
  return PassVar(obj, symbol_table_stack());
}

ObjectPtr NullType::Constructor(Executor*, Args&& params, KWArgs&&) {
  if (params.size() > 0) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                       boost::format("null_t() takes no arguments"));
  }

  ObjectFactory obj_factory(symbol_table_stack());
  return ObjectPtr(obj_factory.NewNull());
}

ObjectPtr BoolType::Constructor(Executor*, Args&& params, KWArgs&&) {
  if (params.size() != 1) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                       boost::format("bool() takes exactly 1 argument"));
  }

  return params[0]->ObjBool();
}

ObjectPtr IntType::Constructor(Executor*, Args&& params, KWArgs&&) {
  if (params.size() != 1) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                       boost::format("int() takes exactly 1 argument"));
  }

  if (params[0]->type() == ObjectType::INT) {
    ObjectFactory obj_factory(symbol_table_stack());
    ObjectPtr obj_int(
        obj_factory.NewInt(static_cast<IntObject&>(*params[0]).value()));

    return obj_int;
  }

  return params[0]->ObjInt();
}

ObjectPtr SliceType::Constructor(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 3, slice)

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewSlice(params[0], params[1], params[2]);
}

ObjectPtr RealType::Constructor(Executor*, Args&& params, KWArgs&&) {
  if (params.size() != 1) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                       boost::format("real() takes exactly 1 argument"));
  }

  if (params[0]->type() == ObjectType::REAL) {
    ObjectFactory obj_factory(symbol_table_stack());

    ObjectPtr obj_real(
        obj_factory.NewReal(static_cast<RealObject&>(*params[0]).value()));

    return obj_real;
  }

  return params[0]->ObjReal();
}

ObjectPtr RangeIterType::Constructor(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS_AT_LEAST(params, 2, range_iter)
  SHPP_FUNC_CHECK_NUM_PARAMS_UNTIL(params, 3, range_iter)

  SHPP_FUNC_CHECK_PARAM_TYPE(params[0], range_iter, INT)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[1], range_iter, INT)

  int step;
  int start = static_cast<IntObject&>(*params[0]).value();
  int end = static_cast<IntObject&>(*params[1]).value();

  if (params.size() == 3) {
    SHPP_FUNC_CHECK_PARAM_TYPE(params[2], range_iter, INT)
    step = static_cast<IntObject&>(*params[2]).value();
  } else {
    if (end > start) {
      step = 1;
    } else {
      step = -1;
    }
  }

  ObjectFactory obj_factory(symbol_table_stack());
  ObjectPtr obj(obj_factory.NewRangeIter(start, end, step));
  return obj;
}

ObjectPtr ArrayIterType::Constructor(Executor*, Args&& params, KWArgs&&) {
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

ObjectPtr MapIterType::Constructor(Executor*, Args&& params, KWArgs&&) {
  if (params.size() != 1) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                       boost::format("array_iter() takes exactly 1 argument"));
  }

  if (params[0]->type() != ObjectType::MAP) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("invalid type for ma_iter"));
  }

  ObjectFactory obj_factory(symbol_table_stack());
  ObjectPtr obj(obj_factory.NewMapIter(params[0]));
  return obj;
}

ObjectPtr CmdIterType::Constructor(Executor*, Args&&, KWArgs&&) {
  throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                     boost::format("cmd_iter is not constructable"));
}

ObjectPtr GlobIterType::Constructor(Executor*, Args&&, KWArgs&&) {
  throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                     boost::format("cmd_iter is not constructable"));
}

ObjectPtr ModuleType::Constructor(Executor*, Args&&, KWArgs&&) {
  throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                     boost::format("module is not constructable"));
}

ObjectPtr TupleType::Constructor(Executor*, Args&& params, KWArgs&&) {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewTuple(std::move(params));
}

ObjectPtr RootObjectType::Constructor(Executor*, Args&&, KWArgs&&) {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewRootObject();
}

static bool InstanceOfIface(ObjectPtr obj, ObjectPtr base) {
  auto ifaces = obj->Interfaces();

  for (auto& iface : ifaces) {
    if (iface.get() == base.get()) {
      return true;
    } else {
      return InstanceOfIface(iface, base);
    }
  }

  return false;
}

static bool InstanceOfBase(ObjectPtr obj, ObjectPtr base) {
  // check base classes
  if (obj.get() == base.get()) {
    return true;
  }

  if (obj->BaseType().get() == base.get()) {
    return true;
  } else {
    if (obj->BaseType()) {
      if (InstanceOfBase(obj->BaseType(), base)) {
        return true;
      }
    }
  }

  if (InstanceOfIface(obj, base)) {
    return true;
  }

  return false;
}

bool InstanceOf(ObjectPtr obj, ObjectPtr base) {
  if (InstanceOfBase(obj->ObjType(), base)) {
    return true;
  }

  if (InstanceOfIface(obj->ObjType(), base)) {
    return true;
  }

  return false;
}

}  // namespace internal
}  // namespace shpp
