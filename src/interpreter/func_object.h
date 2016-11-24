#ifndef SETI_FUNC_OBJ_H
#define SETI_FUNC_OBJ_H

#include <string>
#include <memory>
#include <unordered_map>
#include <tuple>
#include <list>

#include "run_time_error.h"
#include "ast/obj_type.h"
#include "ast/symbol_table.h"
#include "stmt_executor.h"

namespace setti {
namespace internal {

class FuncDeclObject: public FuncObject {
 public:
  FuncDeclObject(const std::string& id, AstNode* start_node,
                 const SymbolTableStack& symbol_table,
                 std::vector<std::string>&& params,
                 std::vector<ObjectPtr>&& default_values,
                 bool variadic, ObjectPtr obj_type)
      : FuncObject(obj_type)
      , id_(id)
      , start_node_(start_node)
      , symbol_table_(true)
      , params_(std::move(params))
      , default_values_(std::move(default_values))
      , variadic_(variadic) {
    symbol_table_.Push(symbol_table.MainTable());
    SymbolTablePtr table = SymbolTable::Create();

    // main symbol of function
    symbol_table_.Push(table, true);
  }

  ObjectPtr Call(Executor* parent, std::vector<ObjectPtr>&& params) override {
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
      ObjectPtr tuple_obj(new TupleObject(std::move(vec_params)));
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

    // Executes the function using the ast nodes
    BlockExecutor executor(parent, symbol_table_);
    executor.Exec(start_node_);

    ObjectPtr obj_ret;
    bool bool_ret = false;
    std::tie(obj_ret, bool_ret) = symbol_table_.LookupObj("%return");

    if (bool_ret) {
      return obj_ret;
    } else {
      return ObjectPtr(new NullObject);
    }
  }

 private:
  std::string id_;
  AstNode* start_node_;
  SymbolTableStack symbol_table_;
  std::vector<std::string> params_;
  std::vector<ObjectPtr> default_values_;
  bool variadic_;
};

}
}

#endif
