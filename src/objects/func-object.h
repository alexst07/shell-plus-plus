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

#ifndef SHPP_FUNC_OBJ_H
#define SHPP_FUNC_OBJ_H

#include <string>
#include <memory>
#include <unordered_map>
#include <tuple>
#include <list>

#include "abstract-obj.h"
#include "run_time_error.h"
#include "obj-type.h"

namespace shpp {
namespace internal {

class FuncType: public TypeObject {
 public:
  FuncType(ObjectPtr obj_type, SymbolTableStack&& sym_table);

  virtual ~FuncType() {}

  ObjectPtr Constructor(Executor*, Args&&, KWArgs&&) override {
      throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                         boost::format("func() not contructable"));

    return ObjectPtr(nullptr);
  }
};

class FuncObject: public Object {
 public:
  FuncObject(ObjectPtr obj_type, SymbolTableStack&& sym_table,
      std::vector<std::string>&& params = std::vector<std::string>(),
      std::vector<std::string>&& default_params = std::vector<std::string>(),
      bool variadic = false)
      : Object(ObjectType::FUNC, obj_type, std::move(sym_table))
      , params_(std::move(params))
      , default_params_(std::move(default_params))
      , variadic_(variadic) {}

  virtual ~FuncObject() {}

  virtual ObjectPtr Params();

  virtual ObjectPtr DefaultParams();

  virtual ObjectPtr Variadic();

  std::size_t Hash() override {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("func object has no hash method"));
  }

  std::string Print() override {
    return std::string("[function]");
  }

 private:
  std::vector<std::string> params_;
  std::vector<std::string> default_params_;
  bool variadic_;
};

class SpecialFuncObject: public Object {
 public:
  SpecialFuncObject(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::SPEC_FUNC, obj_type, std::move(sym_table)) {}

  virtual ~SpecialFuncObject() {}

  std::size_t Hash() override {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("func object has no hash method"));
  }

  virtual ObjectPtr SpecialCall(Executor* parent, Args&& params,
                                KWArgs&& kw_params,
                                SymbolTableStack& curret_sym_tab) = 0;

  std::string Print() override {
    return std::string("[function]");
  }
};

class FuncWrapperObject: public FuncObject {
 public:
  FuncWrapperObject(ObjectPtr obj_type, ObjectPtr func, ObjectPtr self,
                    SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , func_(func)
      , self_(self) {}

  virtual ~FuncWrapperObject() {}

  ObjectPtr Call(Executor* parent, Args&& params, KWArgs&& kw_params) override;

 private:
  ObjectPtr func_;
  ObjectPtr self_;
};

class FuncDeclObject: public FuncObject {
 public:
  FuncDeclObject(const std::string& id, std::shared_ptr<Block> start_node,
      const SymbolTableStack& symbol_table,
      std::vector<std::string>&& params,
      std::unordered_map<std::string, ObjectPtr>&& default_values,
      bool variadic, bool lambda, ObjectPtr obj_type,
      SymbolTableStack&& sym_table);

  ObjectPtr Call(Executor* parent, Args&& params, KWArgs&& kw_params) override;

  ObjectPtr Attr(ObjectPtr self, const std::string& name) override;

 private:
  ObjectPtr Params() override;

  ObjectPtr DefaultParams() override;

  ObjectPtr Variadic() override;

  void HandleArguments(Args&& params, KWArgs&& kw_params);

  void HandleSimpleArguments(Args&& params, KWArgs&& kw_params);

  void HandleVariadicArguments(Args&& params, KWArgs&& kw_params);

  bool CheckParamsInInterval(const std::string& param, size_t begin, size_t end);

  ObjectPtr GetDefaultParam(const std::string& param);

  bool CheckInDefaultValues(const std::string& param);

  void FillParam(std::vector<bool>& params_fill, const std::string& param);

  bool CheckParamIsFill(std::vector<bool>& params_fill, const std::string& param);

  std::string id_;

  // the start_node_ is a pointer on ast, on interactive mode the
  // ast can be free and the func object be keep on symbol table
  // so shared_ptr avoid lost this pointer
  std::shared_ptr<Block> start_node_;
  SymbolTableStack symbol_table_;
  std::vector<std::string> params_;
  std::unordered_map<std::string, ObjectPtr> default_values_;
  bool variadic_;
  bool lambda_;
};

}
}

#endif  // SHPP_FUNC_OBJ_H
