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

#include "decl-class-object.h"

#include "object-factory.h"

namespace shpp {
namespace internal {

AbstractMethod::AbstractMethod(const AbstractMethod& method)
  : num_params_(method.num_params_)
  , num_default_params_(method.num_default_params_)
  , variadic_(method.variadic_) {}

AbstractMethod& AbstractMethod::operator=(const AbstractMethod& method) {
  num_params_ = method.num_params_;
  num_default_params_ = method.num_default_params_;
  variadic_ = method.variadic_;

  return *this;
}

AbstractMethod::AbstractMethod(AbstractMethod&& method)
  : num_params_(method.num_params_)
  , num_default_params_(method.num_default_params_)
  , variadic_(method.variadic_) {}

AbstractMethod& AbstractMethod::operator=(AbstractMethod&& method) {
  num_params_ = method.num_params_;
  num_default_params_ = method.num_default_params_;
  variadic_ = method.variadic_;

  return *this;
}

bool AbstractMethod::operator==(const FuncObject& func) const {
  if (variadic_) {
    return (func.NumParams() == num_params_) &&
         (func.NumDefaultParams() == num_default_params_) &&
         func.CVariadic() == variadic_;
  }

  // the number of params include the number of default_params
  return (func.NumParams() == num_params_) &&
         (func.CVariadic() == variadic_);
}

bool AbstractMethod::operator!=(const FuncObject& func) const {
  return !this->operator==(func);
}

bool AbstractMethod::operator==(const AbstractMethod& func) const {
  if (variadic_) {
    return (func.num_params_ == num_params_) &&
         (func.num_default_params_ == num_default_params_) &&
         func.variadic_ == variadic_;
  }

  // the number of params include the number of default_params
  return (func.num_params_ == num_params_) &&
         (func.variadic_ == variadic_);
}

bool AbstractMethod::operator!=(const AbstractMethod& func) const {
  return !this->operator==(func);
}

DeclClassType::DeclClassType(const std::string& name, ObjectPtr obj_type,
    SymbolTableStack&& sym_table, ObjectPtr base,
    InterfacesList&& ifaces, bool abstract, bool is_final)
    : TypeObject(name, obj_type, std::move(sym_table), base, std::move(ifaces),
          ObjectType::TYPE, is_final)
    , abstract_(abstract) {
  symbol_table_stack().Push(SymbolTablePtr(new SymbolTable(
      SymbolTable::TableType::CLASS_TABLE)));

  // if there is no base type, so we don't need verify abstract methods
  // from base class
  if (!base) {
    return;
  }

  // the class TypeObject already check if the base is a type, so we can
  // cast this object to TypeObject without any problem with segfault
  TypeObject& type_base = static_cast<TypeObject&>(*base);

  if (type_base.is_final()) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
        boost::format("'%1%' can't extends from final type '%2%'")
        %name%type_base.ObjectName());
  }

  // only user declared class has abstract methods
  if (!type_base.Declared()) {
    return;
  }

  // here we are surely that the object is a declared class
  DeclClassType& decl_base = static_cast<DeclClassType&>(*base);

  // insert the abstract methods from the base class
  for (auto& method: decl_base.AbstractMethods()) {
    auto it = abstract_methods_.find(method.first);

    // abstract method must be have unique name
    if (it != abstract_methods_.end()) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
          boost::format("not allowed same name '%1%' method on class")
          %method.first);
    }

    abstract_methods_.insert(std::pair<std::string, AbstractMethod>(method.first,
        method.second));
  }
}

void DeclClassType::AddAbstractMethod(const std::string& name,
    AbstractMethod&& method) {
  // abstract class can not be instantiated
  if (!abstract_) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
        boost::format("not allowed abstract '%1%' method on no abstract class")
        %name);
  }

  // not allowed insert methods with same names
  auto it = abstract_methods_.find(name);
  if (it != abstract_methods_.end()) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
        boost::format("not allowed same name '%1%' method on class")
        %name);
  }

  // verify is exists some impelemented method with same name
  if (ExistsAttr(name)) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
        boost::format("not allowed same name '%1%' attribute on class")
        %name);
  }

  abstract_methods_.insert(std::pair<std::string, AbstractMethod>(name,
      std::move(method)));
}

void DeclClassType::CheckAbstractMethodsCompatibility() {
  // if it is an abstract class, the class doesn't need
  // implement abstract methods from this or from
  // the base class
  if (abstract_) {
    return;
  }

  // verify if all abstract methods are implemented
  for (auto& method: AbstractMethods()) {
    ObjectPtr fobj = SearchAttr(method.first);

    // check if the attribute is really a method
    if (fobj->type() != ObjectType::FUNC) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
          boost::format("attribute '%1%' is not a method")%method.first);
    }

    // check if the abstract method is equal the implemented
    if (method.second != static_cast<FuncObject&>(*fobj)) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
          boost::format("method '%1%' has wrong number of parameters")
          %method.first);
    }
  }
}

void DeclClassType::CheckInterfaceCompatibility() {
  // verify if all methods from interfaces are implemented
  for (auto& iface: Interfaces()) {
    if (iface->type() != ObjectType::DECL_IFACE) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
          boost::format("only interface supported"));
    }

    for (auto& method: static_cast<DeclInterface&>(*iface).Methods()) {
      // if the method in the interface was declared as abstract
      // we don't need to check if the method was implemented
      // in this class, because some derived class has to
      // implement it
      auto it = abstract_methods_.find(method.first);
      if (it != abstract_methods_.end()) {
        if (it->second == method.second) {
          continue;
        }
      }

      // search if the abstract method from interface was implemented
      // on this class or in any class which this class is derived
      ObjectPtr fobj = SearchAttr(method.first);

      if (fobj->type() != ObjectType::FUNC) {
        throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
            boost::format("attribute '%1%' is not a method")%method.first);
      }

      // check if the method in interface is equal the implemented
      if (method.second != static_cast<FuncObject&>(*fobj)) {
        throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
            boost::format("method '%1%' has wrong number of parameters")%method.first);
      }
    }
  }
}

// constructor for declared class call __init__ method from
// symbol table, and create an DeclClassObject, this object
// has a symbol table stack to store attributes
ObjectPtr DeclClassType::Constructor(Executor* parent, Args&& params,
    KWArgs&& kw_params) {
  if (abstract_) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
        boost::format("abstract class '%1%' can not be instantiated")
        %ObjectName());
  }

  ObjectFactory obj_factory(symbol_table_stack());
  ObjectPtr obj_self(obj_factory.NewDeclObject(this->name()));

  if (symbol_table_stack().Exists("__init__")) {
    ObjectPtr obj_init = symbol_table_stack().Lookup("__init__", false)
      .SharedAccess();

    if (obj_init->type() == ObjectType::FUNC) {
      params.insert(params.begin(), obj_self);
      static_cast<FuncObject&>(*obj_init).Call(parent, std::move(params),
          std::move(kw_params));
    }
  }

  return obj_self;
}

ObjectPtr DeclClassType::CallObject(const std::string& name,
                                 ObjectPtr self_param) {
  // search on this class and all base classes
  ObjectPtr obj = SearchAttr(name);

  if (obj->type() == ObjectType::FUNC) {
    ObjectFactory obj_factory(symbol_table_stack());

    // the function wrapper insert the object self_param as the first param
    // it works like self argument
    return ObjectPtr(obj_factory.NewWrapperFunc(obj, self_param));
  }

  return obj;
}

std::shared_ptr<Object>& DeclClassType::AttrAssign(
    std::shared_ptr<Object> /*self*/, const std::string& name) {
  ObjectPtr& att_obj = symbol_table_stack().Lookup(name, true).Ref();

  return att_obj;
}

std::shared_ptr<Object> DeclClassType::Attr(std::shared_ptr<Object>,
    const std::string& name) {
  auto att_obj = SearchAttr(name);
  return PassVar(att_obj, symbol_table_stack());
}

std::shared_ptr<Object> DeclClassObject::Attr(std::shared_ptr<Object> self,
                              const std::string& name) {
  ObjectFactory obj_factory(symbol_table_stack());

  // first check it the attribute exists on object symbol table
  if (symbol_table_stack().Exists(name)) {
    auto att_obj = symbol_table_stack().Lookup(name, false).Ref();

    // functions on object are handle to insert this parameter
    if (att_obj->type() == ObjectType::FUNC) {
      return ObjectPtr(obj_factory.NewWrapperFunc(att_obj, self));
    }

    return PassVar(att_obj, symbol_table_stack());
  }

  // if the attribute is not on object symbol table search it on type class
  // and all base class
  ObjectPtr att_obj = static_cast<TypeObject&>(*ObjType()).SearchAttr(name);

  if (att_obj->type() == ObjectType::FUNC) {
    // if the function is not declared, just return it
    if (!static_cast<FuncObject&>(*att_obj).Declared()) {
      return ObjectPtr(obj_factory.NewWrapperFunc(att_obj, self));
    }

    // if the function is static don't wrapper the function to pass the
    // parameter this
    if (static_cast<FuncDeclObject&>(*att_obj).Static()) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
            boost::format("static method '%1%' must not be called by object")%name);
    }

    // the function wrapper insert the object self_param as the first param
    // it works like self argument
    return ObjectPtr(obj_factory.NewWrapperFunc(att_obj, self));
  }

  return att_obj;
}

std::shared_ptr<Object>& DeclClassObject::AttrAssign(
    std::shared_ptr<Object> /*self*/, const std::string& name) {
  ObjectPtr& att_obj = symbol_table_stack().Lookup(name, true).Ref();

  return att_obj;
}

ObjectPtr DeclClassObject::Add(ObjectPtr obj) {
  Args args = {obj};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__add__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::Sub(ObjectPtr obj) {
  Args args = {obj};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__sub__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::Mult(ObjectPtr obj) {
  Args args = {obj};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__mul__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::Div(ObjectPtr obj) {
  Args args = {obj};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__div__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::DivMod(ObjectPtr obj) {
  Args args = {obj};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__mod__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::RightShift(ObjectPtr obj) {
  Args args = {obj};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__rshift__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::LeftShift(ObjectPtr obj) {
  Args args = {obj};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__lshift__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::Lesser(ObjectPtr obj) {
  Args args = {obj};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__lt__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::Greater(ObjectPtr obj) {
  Args args = {obj};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__gt__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::LessEqual(ObjectPtr obj) {
  Args args = {obj};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__le__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::GreatEqual(ObjectPtr obj) {
  Args args = {obj};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__ge__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::Equal(ObjectPtr obj) {
  Args args = {obj};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__eq__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::In(ObjectPtr obj) {
  Args args = {obj};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__contains__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::NotEqual(ObjectPtr obj) {
  Args args = {obj};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__ne__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::BitAnd(ObjectPtr obj) {
  Args args = {obj};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__rand__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::BitOr(ObjectPtr obj) {
  Args args = {obj};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__ror__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::BitXor(ObjectPtr obj) {
  Args args = {obj};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__rxor__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::BitNot() {
  Args args = {};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__rinvert__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::And(ObjectPtr obj) {
  Args args = {obj};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__and__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::Or(ObjectPtr obj) {
  Args args = {obj};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__or__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::GetItem(ObjectPtr obj) {
  Args args = {obj};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__getitem__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::ObjIter(ObjectPtr /*obj*/) {
  Args args = {};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__iter__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

void DeclClassObject::DelItem(ObjectPtr /*obj*/) {
  Args args = {};
  KWArgs kw_args = {};
  Attr(self_.lock(), "__del__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::UnaryAdd() {
  Args args = {};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__pos__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::UnarySub() {
  Args args = {};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__neg__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::Not() {
  Args args = {};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__invert__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::Begin() {
  Args args = {};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__begin__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::End() {
  Args args = {};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__end__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::Next() {
  Args args = {};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__next__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::HasNext() {
  Args args = {};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__has_next__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::Call(Executor*, Args&& params, KWArgs&& kw_params) {
  return Attr(self_.lock(), "__call__")->Call(nullptr, std::move(params),
      std::move(kw_params));
}

std::string DeclClassObject::Print() {
  Args args = {};
  KWArgs kw_args = {};
  ObjectPtr obj = Attr(self_.lock(), "__print__")->Call(nullptr,
      std::move(args), std::move(kw_args));

  if (obj->type() != ObjectType::STRING) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                      boost::format("print func must return string"));
  }

  return static_cast<StringObject&>(*obj).value();
}

long int DeclClassObject::Len() {
  Args args = {};
  KWArgs kw_args = {};
  ObjectPtr obj = Attr(self_.lock(), "__len__")->Call(nullptr,
      std::move(args), std::move(kw_args));

  if (obj->type() != ObjectType::INT) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                      boost::format("__len__ func must return integer"));
  }

  return static_cast<IntObject&>(*obj).value();
}

std::size_t DeclClassObject::Hash() {
  Args args = {};
  KWArgs kw_args = {};
  ObjectPtr obj = Attr(self_.lock(), "__hash__")->Call(nullptr,
      std::move(args), std::move(kw_args));

  if (obj->type() != ObjectType::INT) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                      boost::format("__hash__ func must return integer"));
  }

  return static_cast<std::size_t>(static_cast<IntObject&>(*obj).value());
}

ObjectPtr DeclClassObject::ObjBool() {
  Args args = {};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__bool__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::ObjCmd() {
  Args args = {};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__cmd__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::ObjString() {
  Args args = {};
  KWArgs kw_args = {};
  return Attr(self_.lock(), "__str__")->Call(nullptr, std::move(args),
      std::move(kw_args));
}

ObjectPtr DeclClassObject::Caller(const std::string& fname, Args&& params,
    KWArgs&& kw_params) {
  SymbolTableStack& st =
      static_cast<DeclClassType&>(*ObjType()).SymTableStack();
  ObjectPtr func_obj = st.Lookup(fname, false).SharedAccess();

  if (func_obj->type() != ObjectType::FUNC) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                      boost::format("symbol %1% must be func")%fname);
  }

  params.insert(params.begin(), self_.lock());

  return static_cast<FuncObject&>(*func_obj).Call(nullptr, std::move(params),
      std::move(kw_params));
}

DeclInterface::DeclInterface(const std::string& name, ObjectPtr obj_type,
    SymbolTableStack&& sym_table,
    std::vector<std::shared_ptr<Object>>&& ifaces)
    : TypeObject(name, obj_type, std::move(sym_table), ObjectPtr(nullptr),
          std::move(ifaces), ObjectType::DECL_IFACE) {
  // insert the methods from the bases interfaces
  // those methods have to be unique
  for (auto& ifc: Interfaces()) {
    for (auto& method: static_cast<DeclInterface&>(*ifc).Methods()) {
      auto it = methods_.find(method.first);
      if (it != methods_.end()) {
        throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
            boost::format("not allowed same name '%1%' method on interface")
            %method.first);
      }

      methods_.insert(std::pair<std::string, AbstractMethod>(method.first,
          method.second));
    }
  }
}

ObjectPtr DeclInterface::Constructor(Executor*, Args&&, KWArgs&&) {
  throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                      boost::format("Interface can not be instantiated"));
}

void DeclInterface::AddMethod(const std::string& name,
    AbstractMethod&& method) {
  // not allowed insert methods with same names
  auto it = methods_.find(name);
  if (it != methods_.end()) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
        boost::format("not allowed same name '%1%' method on interface")
        %name);
  }

  methods_.insert(std::pair<std::string, AbstractMethod>(name,
      std::move(method)));
}

std::shared_ptr<Object> DeclInterface::Attr(std::shared_ptr<Object>,
    const std::string&) {
  throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
      boost::format("Methods from interface can't be called"));
}

}
}
