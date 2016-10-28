#ifndef SETI_EXPR_EXECUTOR_H
#define SETI_EXPR_EXECUTOR_H

#include <string>
#include <memory>
#include <unordered_map>
#include <tuple>

#include "ast/ast.h"

namespace setti {
namespace internal {

class Executor {
 public:
  virtual void Exec() = 0;
};

class ExpressionExecutor: public Executor {
 public:
  virtual void Exec() = 0;
}

class LiteralExecutor {
 public:
  LiteralExecutor(Literal* literal);

 private:
  Literal* literal_;
};

}
}

#endif  // SETI_EXPR_EXECUTOR_H


