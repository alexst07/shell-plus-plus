#ifndef SETI_FUNC_OBJ_H
#define SETI_FUNC_OBJ_H

#include <string>
#include <memory>
#include <unordered_map>
#include <tuple>
#include <list>

#include "run_time_error.h"
#include "obj_type.h"

namespace setti {
namespace internal {

class FuncObject: public Object {
 public:
  FuncObject(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::FUNC, obj_type, std::move(sym_table)) {}

  virtual ~FuncObject() {}

  std::size_t Hash() const override {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("func object has no hash method"));
  }

  bool operator==(const Object& obj) const override {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("func object has no compare method"));
  }

  virtual ObjectPtr Call(Executor* parent, std::vector<ObjectPtr>&& params) = 0;

  void Print() override {
    std::cout << "FUNC";
  }
};

class FuncWrapperObject: public FuncObject {
 public:
  FuncWrapperObject(ObjectPtr obj_type, ObjectPtr func, ObjectPtr self,
                    SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , func_(func)
      , self_(self) {}

  virtual ~FuncWrapperObject() {}

  ObjectPtr Call(Executor* parent, std::vector<ObjectPtr>&& params) override;

 private:
  ObjectPtr func_;
  ObjectPtr self_;
};

class FuncDeclObject: public FuncObject {
 public:
  FuncDeclObject(const std::string& id, AstNode* start_node,
                 const SymbolTableStack& symbol_table,
                 std::vector<std::string>&& params,
                 std::vector<ObjectPtr>&& default_values,
                 bool variadic, ObjectPtr obj_type,
                 SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , id_(id)
      , start_node_(start_node)
      , symbol_table_()
      , params_(std::move(params))
      , default_values_(std::move(default_values))
      , variadic_(variadic) {
    symbol_table_.Push(symbol_table.MainTable(), true);
  }

  ObjectPtr Call(Executor* parent, std::vector<ObjectPtr>&& params) override;

  void HandleArguments(std::vector<ObjectPtr>&& params);

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

#endif  // SETI_FUNC_OBJ_H
