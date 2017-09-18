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

#include "func-object.h"

#include <string>
#include <boost/variant.hpp>

#include "object-factory.h"
#include "interpreter/scope-executor.h"
#include "interpreter/stmt-executor.h"
#include "utils/scope-exit.h"
#include "utils/check.h"

namespace shpp {
namespace internal {

FuncType::FuncType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("function", obj_type, std::move(sym_table)) {}

ObjectPtr FuncObject::Params() {
  ObjectFactory obj_factory(symbol_table_stack());
  std::vector<ObjectPtr> array;

  for (auto& p: params_) {
    array.push_back(obj_factory.NewString(p));
  }

  return obj_factory.NewArray(std::move(array));
}

ObjectPtr FuncObject::DefaultParams() {
  ObjectFactory obj_factory(symbol_table_stack());
  std::vector<ObjectPtr> array;

  for (auto& p: default_params_) {
    array.push_back(obj_factory.NewString(p));
  }

  return obj_factory.NewArray(std::move(array));
}

ObjectPtr FuncObject::Variadic() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(variadic_);
}

ObjectPtr FuncWrapperObject::Call(Executor* parent, Args&& params, KWArgs&&) {
  FuncObject& func_obj = static_cast<FuncObject&>(*func_);
  params.insert(params.begin(), self_);
  return func_obj.Call(parent, std::move(params));
}

FuncDeclObject::FuncDeclObject(const std::string& id,
    std::shared_ptr<Block> start_node,
    const SymbolTableStack& symbol_table,
    std::vector<std::string>&& params,
    std::unordered_map<std::string, ObjectPtr>&& default_values,
    bool variadic, bool lambda, bool fstatic, ObjectPtr obj_type,
    SymbolTableStack&& sym_table)
    : FuncObject(obj_type, std::move(sym_table),
          std::move(std::vector<std::string>()),
          std::move(std::vector<std::string>()),
          false, true)
    , id_(id)
    , start_node_(start_node)
    , symbol_table_{lambda?symbol_table:SymbolTableStack()}
    , params_(std::move(params))
    , default_values_(std::move(default_values))
    , variadic_(variadic)
    , lambda_(lambda)
    , fstatic_(fstatic){
  if (lambda) {
    // if is lambda create a new symbol table on stack, it works like if stmt
    symbol_table_.NewTable();
  } else {
    // if is function declaration get only the first symbol table
    symbol_table_.Push(symbol_table.MainTable(), true);
  }

  symbol_table_stack().SetEntry("__params__", Params());
  symbol_table_stack().SetEntry("__default_params__", DefaultParams());
  symbol_table_stack().SetEntry("__variadic__", Variadic());
}

ObjectPtr FuncDeclObject::Attr(ObjectPtr self, const std::string& name) {
  ObjectPtr obj_type = ObjType();
  return static_cast<TypeObject&>(*obj_type).CallObject(name, self);
}

void FuncDeclObject::HandleSimpleArguments(Args&& params, KWArgs&& kw_params) {
  size_t no_values_params = params_.size() - default_values_.size();
  size_t params_size = params.size() + kw_params.size();
  std::vector<bool> params_fill(params_.size(), false);

  if ((params_size < (params_.size() - default_values_.size())) ||
      params_size > params_.size()) {
    boost::format msg_error;
    if (default_values_.size() > 0) {
      msg_error = boost::format("%1% takes at least %2% argument "
                                "(%3% given)")%
                                id_% no_values_params % params_size;
    } else {
      msg_error = boost::format("%1% takes exactly %2% argument (%3% given)")%
                                id_% params_.size() % params_size;
    }

    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS, msg_error);
  }

  // Insert objects on symbol table
  for (size_t i = 0; i < params.size(); i++) {
    symbol_table_.SetEntry(params_[i], params[i]);
    params_fill[i] = true;
  }

  size_t endps = params_.size() > 0?params_.size():0;
  size_t endp = params.size() > 0?params.size():0;

  for (auto& kw_param: kw_params) {
    if (CheckParamsInInterval(kw_param.first, 0, endp)) {
      throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
          boost::format("got multiple values for argument '%1%'")
          %kw_param.first);
    }

    if (!CheckParamsInInterval(kw_param.first, 0, endps)) {
      throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
          boost::format("got an unexpected keyword argument '%1%'")
          %kw_param.first);
    }

    symbol_table_.SetEntry(kw_param.first, kw_param.second);
    FillParam(params_fill, kw_param.first);
  }

  for (size_t i = 0; i < params_.size(); i++) {
    if (params_fill[i] == false) {
      symbol_table_.SetEntry(params_[i], GetDefaultParam(params_[i]));
      params_fill[i] = true;
    }
  }

  for (auto& default_value: default_values_) {
    if (!CheckParamIsFill(params_fill, default_value.first)) {
      symbol_table_.SetEntry(default_value.first, default_value.second);
    }
  }
}

void FuncDeclObject::HandleVariadicArguments(Args&& params,
    KWArgs&& kw_params) {
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

  for (auto& default_value: default_values_) {
    symbol_table_.SetEntry(default_value.first, default_value.second);
  }

  for (auto& kw_param: kw_params) {
    if (!CheckInDefaultValues(kw_param.first)) {
      throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
            boost::format("argument '%1%' is not default value")
            %kw_param.first);
    }

    symbol_table_.SetEntry(kw_param.first, kw_param.second);
  }
}

void FuncDeclObject::HandleArguments(Args&& params, KWArgs&& kw_params) {
  if (variadic_) {
    HandleVariadicArguments(std::move(params), std::move(kw_params));
  } else {
    HandleSimpleArguments(std::move(params), std::move(kw_params));
  }
}

ObjectPtr FuncDeclObject::GetDefaultParam(const std::string& param) {
  try {
    return default_values_[param];
  } catch (std::out_of_range&) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
              boost::format("keyword argument '%1%' not found")
              %param);
  }
}

bool FuncDeclObject::CheckParamsInInterval(const std::string& param,
    size_t begin, size_t end) {
  for (size_t i = begin; i < end; i++) {
    if (params_[i] == param) {
      return true;
    }
  }

  return false;
}

void FuncDeclObject::FillParam(std::vector<bool>& params_fill,
    const std::string& param) {
  for (size_t i = 0; i < params_.size(); i++) {
    if (params_[i] == param) {
      params_fill[i] = true;
      break;
    }
  }
}

bool FuncDeclObject::CheckParamIsFill(std::vector<bool>& params_fill,
    const std::string& param) {
  for (size_t i = 0; i < params_.size(); i++) {
    if (params_[i] == param) {
      if (params_fill[i] == true) {
        return true;
      }

      break;
    }
  }

  return false;
}

bool FuncDeclObject::CheckInDefaultValues(const std::string& param) {
  auto it = default_values_.find(param);

  if (it == default_values_.end()) {
    return false;
  }

  return true;
}

ObjectPtr FuncDeclObject::Call(Executor* parent, Args&& params,
                               KWArgs&& kw_params) {
  SymbolTable::TableType table_type = lambda_?
      SymbolTable::TableType::LAMBDA_TABLE: SymbolTable::TableType::FUNC_TABLE;

  // it is the table function
  SymbolTablePtr table = SymbolTable::Create(table_type);

  // main symbol of function
  symbol_table_.Push(table, false);

  BlockExecutor executor(parent, symbol_table_, true);

  // scope exit case an excpetion thrown
  auto cleanup = MakeScopeExit([&]() {
    executor.ExecuteDeferStack();
    symbol_table_.Pop();
  });
  IgnoreUnused(cleanup);

  HandleArguments(std::move(params), std::move(kw_params));

  // Executes the function using the ast nodes
  executor.Exec(start_node_.get());

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

ObjectPtr FuncDeclObject::Params() {
  ObjectFactory obj_factory(symbol_table_stack());
  std::vector<ObjectPtr> array;

  for (auto& p: params_) {
    array.push_back(obj_factory.NewString(p));
  }

  return obj_factory.NewArray(std::move(array));
}

ObjectPtr FuncDeclObject::DefaultParams() {
  ObjectFactory obj_factory(symbol_table_stack());
  std::vector<ObjectPtr> array;

  for (auto& p: default_values_) {
    array.push_back(obj_factory.NewString(p.first));
  }

  return obj_factory.NewArray(std::move(array));
}

ObjectPtr FuncDeclObject::Variadic() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(variadic_);
}

}
}
