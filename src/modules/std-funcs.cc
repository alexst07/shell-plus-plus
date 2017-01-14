#include "std-funcs.h"

#include "utils/check.h"

namespace setti {
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

ObjectPtr LenFunc::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1, len)

  long int size = params[0]->Len();

  return obj_factory_.NewInt(static_cast<int>(size));
}

}
}
}
}
