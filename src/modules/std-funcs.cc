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

#include "std-funcs.h"

#include "utils/check.h"
#include "utils/glob.h"

namespace shpp {
namespace internal {
namespace module {
namespace stdf {

ObjectPtr PrintFunc::Call(Executor*, std::vector<ObjectPtr>&& params) {
  for (auto& e: params) {
    std::cout << e->Print();
  }

  std::cout << "\n";

  return obj_factory_.NewNull();
}

ObjectPtr PrintErrFunc::Call(Executor*, std::vector<ObjectPtr>&& params) {
  for (auto& e: params) {
    std::cerr << e->Print();
  }

  std::cerr << "\n";

  return obj_factory_.NewNull();
}

ObjectPtr ReadFunc::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_PARAM_TYPE(params[0], path, STRING)

  std::string str;

  std::cout << static_cast<StringObject&>(*params[0]).value();
  std::cin >> str;

  return obj_factory_.NewString(str);
}

ObjectPtr LenFunc::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, len)

  long int size = params[0]->Len();

  return obj_factory_.NewInt(static_cast<int>(size));
}

ObjectPtr CompFunc::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 2, comp)

  ObjectPtr obj_resp = params[0]->Lesser(params[1]);

  if (obj_resp->type() != Object::ObjectType::BOOL) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("operator less must return bool"));
  }

  return obj_resp;
}

ObjectPtr RangeFunc::Call(Executor*, std::vector<ObjectPtr>&& params) {
  ObjectPtr obj = obj_factory_.NewRangeIterType();
  RangeIterType& range_it_type = static_cast<RangeIterType&>(*obj);
  return range_it_type.Constructor(nullptr, std::move(params));
}

ObjectPtr AssertFunc::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 2, assert)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[0], test, BOOL)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[1], msg, STRING)

  bool v = static_cast<BoolObject&>(*params[0]).value();
  std::string msg = static_cast<StringObject&>(*params[1]).value();

  if (!v) {
    throw RunTimeError(RunTimeError::ErrorCode::ASSERT,
                       boost::format(msg));
  }

  return obj_factory_.NewNull();
}

ObjectPtr IsInteractiveFunc::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 0, params)

  int shell_terminal;
  int shell_is_interactive;

  // see if we are running interactively
  shell_terminal = STDIN_FILENO;
  shell_is_interactive = isatty(shell_terminal);

  bool v = shell_is_interactive?true:false;

  return obj_factory_.NewBool(v);
}

ObjectPtr GlobFunc::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, params)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[0], msg, STRING)

  ObjectFactory obj_factory(symbol_table_stack());
  const std::string& glob_str = static_cast<StringObject&>(*params[0]).value();

  std::vector<ObjectPtr> glob_obj = ExecGlob(glob_str, symbol_table_stack());
  return obj_factory.NewArray(std::move(glob_obj));
}

ObjectPtr GlobRFunc::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, params)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[0], msg, STRING)

  ObjectFactory obj_factory(symbol_table_stack());
  const std::string& glob_str = static_cast<StringObject&>(*params[0]).value();

  std::vector<ObjectPtr> glob_obj =
      ListTree(boost::filesystem::current_path(), glob_str,
      symbol_table_stack());

  return obj_factory.NewArray(std::move(glob_obj));
}

}
}
}
}
