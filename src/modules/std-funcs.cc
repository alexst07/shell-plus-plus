#include "std-funcs.h"


namespace setti {
namespace internal {

ObjectPtr PrintFunc::Call(Executor*, std::vector<ObjectPtr>&& params) {
  for (auto& e: params) {
    e->Print();
  }

  std::cout << "\n";

  return obj_factory_.NewNull();
}

}
}
