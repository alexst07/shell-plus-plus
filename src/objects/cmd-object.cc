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

#include "cmd-object.h"

#include <boost/algorithm/string.hpp>
#include <boost/variant.hpp>
#include <string>

#include "obj-type.h"
#include "object-factory.h"
#include "utils/check.h"

namespace shpp {
namespace internal {

CmdIterObject::CmdIterObject(std::string delim, int outerr, ObjectPtr cmd_obj,
                             ObjectPtr obj_type, SymbolTableStack&& sym_table)
    : BaseIter(ObjectType::CMD_ITER, obj_type, std::move(sym_table)), pos_(0) {
  if (cmd_obj->type() != ObjectType::CMD) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("only cmdobj supported"));
  }

  cmd_obj_ = cmd_obj;
  CmdObject& cmd_obj_ref = static_cast<CmdObject&>(*cmd_obj_);

  std::string str_cmd;

  if (outerr == 0) {
    str_cmd = cmd_obj_ref.str_stdout();
  } else {
    str_cmd = cmd_obj_ref.str_stderr();
  }

  boost::trim_if(str_cmd, boost::is_any_of(delim));

  if (str_cmd.empty()) {
    return;
  }

  boost::algorithm::split(str_split_, str_cmd, boost::is_any_of(delim),
                          boost::algorithm::token_compress_on);
}

ObjectPtr CmdIterObject::Next() {
  std::string str = str_split_.at(pos_++);

  ObjectFactory obj_factory(symbol_table_stack());
  ObjectPtr obj_str(obj_factory.NewString(std::move(str)));

  return obj_str;
}

ObjectPtr CmdIterObject::HasNext() {
  ObjectFactory obj_factory(symbol_table_stack());

  bool v = pos_ == str_split_.size();
  return obj_factory.NewBool(!v);
}

ObjectPtr CmdObject::ObjIter(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewCmdIter(delim_, 0, obj);
}

ObjectPtr CmdObject::ObjString() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewString(str_stdout());
}

ObjectPtr CmdObject::ObjBool() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(status_ == 0 ? true : false);
}

ObjectPtr CmdObject::Not() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(status_ == 0 ? false : true);
}

ObjectPtr CmdObject::ObjArray() {
  ObjectFactory obj_factory(symbol_table_stack());

  std::string str_cmd = str_stdout_;
  boost::trim_if(str_cmd, boost::is_any_of(delim_));

  Args arr_obj;
  std::vector<std::string> arr_str;

  if (str_cmd.empty()) {
    return obj_factory.NewArray(std::move(arr_obj));
  }

  boost::algorithm::split(arr_str, str_cmd, boost::is_any_of(delim_),
                          boost::algorithm::token_compress_on);

  for (auto& s : arr_str) {
    arr_obj.push_back(obj_factory.NewString(s));
  }

  return obj_factory.NewArray(std::move(arr_obj));
}

std::shared_ptr<Object> CmdObject::Attr(std::shared_ptr<Object> self,
                                        const std::string& name) {
  ObjectPtr obj_type = ObjType();
  return static_cast<TypeObject&>(*obj_type).CallObject(name, self);
}

ObjectPtr CmdObject::ObjCmd() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewString(str_stdout_);
}

bool CmdObject::Compare(ObjectPtr obj) {
  ObjectPtr obj_type = obj->ObjType();

  TypeObject& type = static_cast<TypeObject&>(*obj_type);

  ObjectFactory obj_factory(symbol_table_stack());

  Args params_array = {obj_factory.NewString(str_stdout_)};
  ObjectPtr obj_comp = type.Constructor(nullptr, std::move(params_array));
  ObjectPtr obj_bool = obj_comp->Equal(obj);

  if (obj_bool->type() == ObjectType::BOOL) {
    bool v = static_cast<BoolObject&>(*obj_bool).value();

    if (v) {
      return true;
    }
  }

  return false;
}

ObjectPtr CmdObject::Equal(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(Compare(obj));
}

ObjectPtr CmdObject::NotEqual(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(!Compare(obj));
}

ObjectPtr CmdObject::In(ObjectPtr obj) {
  ObjectPtr obj_type = obj->ObjType();

  TypeObject& type = static_cast<TypeObject&>(*obj_type);

  ObjectFactory obj_factory(symbol_table_stack());

  std::string str_cmd = str_stdout_;
  boost::trim_if(str_cmd, boost::is_any_of(delim_));

  Args arr_obj;
  std::vector<std::string> arr_str;

  if (str_cmd.empty()) {
    return obj_factory.NewBool(false);
  }

  boost::algorithm::split(arr_str, str_cmd, boost::is_any_of(delim_),
                          boost::algorithm::token_compress_on);

  for (auto& s : arr_str) {
    Args params_array = {obj_factory.NewString(s)};
    ObjectPtr obj_comp = type.Constructor(nullptr, std::move(params_array));
    ObjectPtr obj_bool = obj_comp->Equal(obj);

    if (obj_bool->type() == ObjectType::BOOL) {
      bool v = static_cast<BoolObject&>(*obj_bool).value();

      if (v) {
        return obj_factory.NewBool(true);
      }
    }
  }

  return obj_factory.NewBool(false);
}

CmdType::CmdType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
    : TypeObject("cmdobj", obj_type, std::move(sym_table)) {
  RegisterMethod<CmdOutFunc>("out", symbol_table_stack(), *this);
  RegisterMethod<CmdErrFunc>("err", symbol_table_stack(), *this);
  RegisterMethod<CmdStatusFunc>("status", symbol_table_stack(), *this);
  RegisterMethod<CmdDelimFunc>("delim", symbol_table_stack(), *this);
}

ObjectPtr CmdType::Constructor(Executor*, Args&&, KWArgs&&) {
  throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                     boost::format("cmdobj is not constructable"));
}

ObjectPtr CmdOutFunc::Call(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, out)

  CmdObject& cmd_obj = static_cast<CmdObject&>(*params[0]);

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewString(cmd_obj.str_stdout());
}

ObjectPtr CmdErrFunc::Call(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, out)

  CmdObject& cmd_obj = static_cast<CmdObject&>(*params[0]);

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewString(cmd_obj.str_stderr());
}

ObjectPtr CmdDelimFunc::Call(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS_UNTIL(params, 2, delim)

  CmdObject& cmd_obj = static_cast<CmdObject&>(*params[0]);

  if (params.size() == 2) {
    SHPP_FUNC_CHECK_PARAM_TYPE(params[1], delim, STRING)
    std::string delim = static_cast<StringObject&>(*params[1]).value();
    cmd_obj.set_delim(delim);
    return params[0];
  }

  std::string delim = cmd_obj.delim();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewString(delim);
}

ObjectPtr CmdStatusFunc::Call(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, out)

  CmdObject& cmd_obj = static_cast<CmdObject&>(*params[0]);

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewInt(cmd_obj.status());
}

}  // namespace internal
}  // namespace shpp
