#ifndef SETI_EXECUTOR_H
#define SETI_EXECUTOR_H

#include <string>
#include <memory>
#include <unordered_map>
#include <tuple>

#include "ast/ast.h"
#include "ast/obj_type.h"

namespace setti {
namespace internal {

class Executor {
 public:
  Executor(Executor* parent): parent_(parent) {}

 protected:
  Executor* parent() const noexcept {
    return parent_;
  }

 private:
  Executor* parent_;
};

}
}

#endif  // SETI_EXECUTOR_H


