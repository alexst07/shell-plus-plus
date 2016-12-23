#include "func_object.h"

#include <string>
#include <boost/variant.hpp>

#include "object-factory.h"
#include "scope-executor.h"
#include "stmt_executor.h"
#include "utils/scope-exit.h"

namespace setti {
namespace internal {

ObjectPtr FuncWrapperObject::Call(Executor* parent,
                                  std::vector<ObjectPtr>&& params) {
  FuncObject& func_obj = static_cast<FuncObject&>(*func_);
  params.insert(params.begin(), self_);
  return func_obj.Call(parent, std::move(params));
}

void FuncDeclObject::HandleArguments(std::vector<ObjectPtr>&& params) {
  if (variadic_) {
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
  } else {
    if ((params.size() < (params_.size() - default_values_.size())) ||
        (params.size() > params_.size())) {
      throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
          boost::format("%1% takes exactly %2% argument (%3% given)")%
                        id_% (params_.size() - 1)% params.size());
    }

    // Insert objects on symbol table
    for (size_t i = 0; i < params.size(); i++) {
      symbol_table_.SetEntry(params_[i], params[i]);
    }

    size_t no_values_params = params_.size() - default_values_.size();

    for (size_t i = no_values_params; i < params_.size(); i++) {
      symbol_table_.SetEntry(params_[i],
                             default_values_[i - no_values_params]);
    }
  }
}

ObjectPtr FuncDeclObject::Call(Executor* parent,
                               std::vector<ObjectPtr>&& params) {
  // it is the table function
  SymbolTablePtr table = SymbolTable::Create(true);

  // main symbol of function
  symbol_table_.Push(table, false);

  BlockExecutor executor(parent, symbol_table_, true);

  // scope exit case an excpetion thrown
  auto cleanup = MakeScopeExit([&]() {
    executor.ExecuteDeferStack();
    symbol_table_.Pop();
  });
  IgnoreUnused(cleanup);

  HandleArguments(std::move(params));

  // Executes the function using the ast nodes
  executor.Exec(start_node_);

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


}
}
