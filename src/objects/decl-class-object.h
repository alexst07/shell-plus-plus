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

#ifndef SHPP_DECL_CLASS_OBJECT_H
#define SHPP_DECL_CLASS_OBJECT_H

#include <string>
#include <memory>

#include "run_time_error.h"
#include "interpreter/symbol-table.h"
#include "abstract-obj.h"
#include "simple-object.h"
#include "func-object.h"

namespace shpp {
namespace internal {

// Auxiliar functions to help call functions with variable number of parameters
template<typename Obj>
std::vector<Obj> PackArgs(Obj arg) {
  return std::vector<Obj>{arg};
}

template<typename Obj, typename ...Objs>
std::vector<Obj> PackArgs(Obj arg, Objs... args) {
  std::vector<Obj> vec_ret = {arg};
  std::vector<Obj> vec = PackArgs(std::forward<Objs>(args)...);
  vec_ret.insert(vec_ret.end(), vec.begin(), vec.end());
  return vec_ret;
}

class AbstractMethod {
 public:
   AbstractMethod(size_t num_params, size_t num_default_param, bool variadic)
      : num_params_(num_params)
      , num_default_params_(num_default_param)
      , variadic_(variadic) {}

  AbstractMethod(FuncObject& fobj)
      : num_params_(fobj.NumParams())
      , num_default_params_(fobj.NumDefaultParams())
      , variadic_(fobj.CVariadic()) {}

  virtual ~AbstractMethod() = default;

  AbstractMethod(const AbstractMethod&);
  AbstractMethod& operator=(const AbstractMethod&);

  AbstractMethod(AbstractMethod&& method);
  AbstractMethod& operator=(AbstractMethod&& method);

  bool operator==(const FuncObject& func) const;
  bool operator!=(const FuncObject& func) const;

 private:
  size_t num_params_;
  size_t num_default_params_;
  bool variadic_;
};

class DeclClassType: public TypeObject {
 public:
  DeclClassType(const std::string& name, ObjectPtr obj_type,
      SymbolTableStack&& sym_table, ObjectPtr base = ObjectPtr(nullptr),
      InterfacesList&& ifaces = InterfacesList())
      : TypeObject(name, obj_type, std::move(sym_table), base,
                   std::move(ifaces)) {
    symbol_table_stack().Push(SymbolTablePtr(new SymbolTable(
        SymbolTable::TableType::CLASS_TABLE)));
  }

  virtual ~DeclClassType() {}

  bool RegiterMethod(const std::string& name, ObjectPtr obj) override {
    SymbolAttr sym_entry(obj, true);
    return symbol_table_stack().InsertEntry(name, std::move(sym_entry));
  }

  ObjectPtr CallObject(const std::string& name, ObjectPtr self_param) override;

  std::shared_ptr<Object> Attr(std::shared_ptr<Object> self,
                               const std::string& name) override;

  SymbolTableStack& SymTableStack() noexcept {
    return symbol_table_stack();
  }

  SymbolTableStack& GlobalSymTableStack() noexcept {
    return symbol_table_stack();
  }

  ObjectPtr Constructor(Executor* parent, Args&& params, KWArgs&&) override;

  void CheckInterfaceCompatibility();
};

class DeclClassObject: public Object {
 public:
  DeclClassObject(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::DECL_OBJ, obj_type, std::move(sym_table)) {
    symbol_table_stack().Push(SymbolTablePtr(new SymbolTable(
        SymbolTable::TableType::CLASS_TABLE)));
  }

  virtual ~DeclClassObject() {}

  std::shared_ptr<Object> Attr(std::shared_ptr<Object> self,
                                const std::string& name) override;

  std::shared_ptr<Object>& AttrAssign(std::shared_ptr<Object>,
                                        const std::string& name) override;

  ObjectPtr Add(ObjectPtr obj) override;

  ObjectPtr Sub(ObjectPtr obj) override;

  ObjectPtr Mult(ObjectPtr obj) override;

  ObjectPtr Div(ObjectPtr obj) override;

  ObjectPtr DivMod(ObjectPtr obj) override;

  ObjectPtr RightShift(ObjectPtr obj) override;

  ObjectPtr LeftShift(ObjectPtr obj) override;

  ObjectPtr Lesser(ObjectPtr obj) override;

  ObjectPtr Greater(ObjectPtr obj) override;

  ObjectPtr Next() override;

  ObjectPtr HasNext() override;

  ObjectPtr LessEqual(ObjectPtr obj) override;

  ObjectPtr GreatEqual(ObjectPtr obj) override;

  ObjectPtr Equal(ObjectPtr obj) override;

  ObjectPtr In(ObjectPtr obj) override;

  ObjectPtr NotEqual(ObjectPtr obj) override;

  ObjectPtr BitAnd(ObjectPtr obj) override;

  ObjectPtr BitOr(ObjectPtr obj) override;

  ObjectPtr BitXor(ObjectPtr obj) override;

  ObjectPtr BitNot() override;

  ObjectPtr And(ObjectPtr obj) override;

  ObjectPtr Or(ObjectPtr obj) override;

  ObjectPtr UnaryAdd() override;

  ObjectPtr UnarySub() override;

  ObjectPtr Not() override;

  ObjectPtr Begin() override;

  ObjectPtr End() override;

  long int Len() override;

  std::size_t Hash() override;

  std::string Print() override;

  ObjectPtr ObjBool() override;

  ObjectPtr ObjString() override;

  ObjectPtr ObjCmd() override;

  ObjectPtr GetItem(ObjectPtr obj) override;

  ObjectPtr ObjIter(ObjectPtr obj) override;

  void DelItem(ObjectPtr obj) override;

  ObjectPtr Call(Executor*, Args&& params, KWArgs&& kw_params) override;

  SymbolTableStack& SymTable() {
    return symbol_table_stack();
  }

  void SetSelf(ObjectPtr self_obj) {
    self_ = self_obj;
  }

 private:
  template<typename Obj, typename ...Objs>
  ObjectPtr Caller(const std::string& fname, Obj arg, Objs... args) {
    SymbolTableStack& st =
        static_cast<DeclClassType&>(*ObjType()).SymTableStack();
    ObjectPtr func_obj = st.Lookup(fname, false).SharedAccess();

    if (func_obj->type() != ObjectType::FUNC) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                        boost::format("symbol %1% must be func")%fname);
    }

    std::vector<ObjectPtr> params = PackArgs(arg, std::forward<Objs>(args)...);

    // empty kw_args, functions using this caller is operator overload
    // functions that have the fix number of parameters and doesn't
    // have keywords params
    KWArgs kw_args;

    return static_cast<FuncObject&>(*func_obj).Call(nullptr,
        std::move(params), std::move(kw_args));
  }

  ObjectPtr Caller(const std::string& fname, Args&& params,
                   KWArgs&& kw_params);

  std::weak_ptr<Object> self_;
};

class DeclInterface: public TypeObject {
 public:
  DeclInterface(const std::string& name, ObjectPtr obj_type,
      SymbolTableStack&& sym_table,
      std::vector<std::shared_ptr<Object>>&& ifaces);

  virtual ~DeclInterface() = default;

  std::shared_ptr<Object> Attr(std::shared_ptr<Object> self,
                               const std::string& name) override;

  void AddMethod(const std::string& name, AbstractMethod&& method);

  SymbolTableStack& SymTableStack() noexcept {
    return symbol_table_stack();
  }

  inline const std::map<std::string, AbstractMethod>& Methods() const noexcept {
    return methods_;
  }

  ObjectPtr Constructor(Executor* parent, Args&& params, KWArgs&&) override;

 private:
  std::map<std::string, AbstractMethod> methods_;
};

}
}

#endif  // SHPP_DECL_CLASS_OBJECT_H
