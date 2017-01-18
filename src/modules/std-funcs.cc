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

namespace seti {
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
  SETI_FUNC_CHECK_PARAM_TYPE(params[0], path, STRING)

  std::string str;

  std::cout << static_cast<StringObject&>(*params[0]).value();
  std::cin >> str;

  return obj_factory_.NewString(str);
}

ObjectPtr LenFunc::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1, len)

  long int size = params[0]->Len();

  return obj_factory_.NewInt(static_cast<int>(size));
}

ObjectPtr AssertFunc::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 2, assert)
  SETI_FUNC_CHECK_PARAM_TYPE(params[0], test, BOOL)
  SETI_FUNC_CHECK_PARAM_TYPE(params[1], msg, STRING)

  bool v = static_cast<BoolObject&>(*params[0]).value();
  std::string msg = static_cast<StringObject&>(*params[1]).value();

  if (!v) {
    throw RunTimeError(RunTimeError::ErrorCode::ASSERT,
                       boost::format(msg));
  }

  return obj_factory_.NewNull();
}

}
}
}
}
