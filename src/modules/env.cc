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

#include "env.h"

#include <cstdlib>

#include "utils/check.h"

namespace shpp {
namespace internal {
namespace module {
namespace env {

ObjectPtr SetFunc::Call(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 2, set)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[0], var, STRING)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[1], value, STRING)

  const std::string& var = static_cast<StringObject&>(*params[0]).value();
  const std::string& value = static_cast<StringObject&>(*params[1]).value();

  // the last parameter set overwrite flag
  int r = setenv(var.c_str(), value.c_str(), 1);

  return obj_factory_.NewBool(r == 0? true: false);
}

ObjectPtr GetFunc::Call(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, get)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[0], var, STRING)

  const std::string& var = static_cast<StringObject&>(*params[0]).value();

  // the last parameter set overwrite flag
  char *r = getenv(var.c_str());
  std::string str_var;

  if (r != nullptr) {
    str_var = r;
  }

  return obj_factory_.NewString(str_var);
}

ObjectPtr ExistsFunc::Call(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, get)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[0], var, STRING)

  const std::string& var = static_cast<StringObject&>(*params[0]).value();

  // the last parameter set overwrite flag
  char *r = getenv(var.c_str());

  bool v = false;

  if (r != nullptr) {
    v = true;
  }

  return obj_factory_.NewBool(v);
}

ObjectPtr AppendFunc::Call(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 2, append)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[0], var, STRING)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[1], value, STRING)

  const std::string& var = static_cast<StringObject&>(*params[0]).value();
  const std::string& value = static_cast<StringObject&>(*params[1]).value();

  // the last parameter set overwrite flag
  char *str_ret = getenv(var.c_str());
  std::string str_value;

  if (str_ret != nullptr) {
    str_value = str_ret;
  }

  str_value += value;

  // the last parameter set overwrite flag
  int r = setenv(var.c_str(), str_value.c_str(), 1);

  return obj_factory_.NewBool(r == 0? true: false);
}

ObjectPtr UnsetFunc::Call(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, get)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[0], var, STRING)

  const std::string& var = static_cast<StringObject&>(*params[0]).value();

  // the last parameter set overwrite flag
  int r = unsetenv(var.c_str());

  return obj_factory_.NewBool(r == 0? true: false);
}

}
}
}
}
