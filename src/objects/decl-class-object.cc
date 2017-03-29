#include "decl-class-object.h"

#include "object-factory.h"

namespace shpp {
namespace internal {

// constructor for declared class call __init__ method from
// symbol table, and create an DeclClassObject, this object
// has a symbol table stack to store attributes
ObjectPtr DeclClassType::Constructor(Executor* parent,
                                     std::vector<ObjectPtr>&& params) {
  ObjectFactory obj_factory(symbol_table_stack());
  ObjectPtr obj_self(obj_factory.NewDeclObject(this->name()));

  ObjectPtr obj_init = symbol_table_stack().Lookup("__init__", false)
      .SharedAccess();

  if (obj_init->type() == ObjectType::FUNC) {
    params.insert(params.begin(), obj_self);
    static_cast<FuncObject&>(*obj_init).Call(parent, std::move(params));
  }

  return obj_self;
}

ObjectPtr DeclClassType::CallObject(const std::string& name,
                                 ObjectPtr self_param) {
  ObjectPtr obj = symbol_table_stack().Lookup(name, false).SharedAccess();

  if (obj->type() == ObjectType::FUNC) {
    ObjectFactory obj_factory(symbol_table_stack());

    // the function wrapper insert the object self_param as the first param
    // it works like self argument
    return ObjectPtr(obj_factory.NewWrapperFunc(obj, self_param));
  }

  return obj;
}

std::shared_ptr<Object> DeclClassType::Attr(std::shared_ptr<Object> self,
                              const std::string& name) {
  ObjectPtr att_obj = symbol_table_stack().Lookup(name, false).SharedAccess();

  return att_obj;
}

std::shared_ptr<Object> DeclClassObject::Attr(std::shared_ptr<Object> self,
                              const std::string& name) {
  if (symbol_table_stack().ExistsSymbolInClass(name)) {
    ObjectPtr att_obj = symbol_table_stack().LookupClass(name).SharedAccess();

    if (att_obj->type() == ObjectType::FUNC) {
      return static_cast<DeclClassType&>(*ObjType()).CallObject(name, self);
    }

    return att_obj;
  }

  SymbolTableStack& st =
      static_cast<DeclClassType&>(*ObjType()).SymTableStack();
  ObjectPtr att_obj = st.Lookup(name, false).SharedAccess();

  if (att_obj->type() == ObjectType::FUNC) {
    return static_cast<DeclClassType&>(*ObjType()).CallObject(name, self);
  }

  return att_obj;
}

std::shared_ptr<Object>& DeclClassObject::AttrAssign(
    std::shared_ptr<Object> /*self*/, const std::string& name) {
  ObjectPtr& att_obj = symbol_table_stack().LookupClass(name).Ref();

  return att_obj;
}

ObjectPtr DeclClassObject::Add(ObjectPtr obj) {
  return Caller("__add__", self_.lock(), obj);
}

ObjectPtr DeclClassObject::Sub(ObjectPtr obj) {
  return Caller("__sub__", self_.lock(), obj);
}

ObjectPtr DeclClassObject::Mult(ObjectPtr obj) {
  return Caller("__mul__", self_.lock(), obj);
}

ObjectPtr DeclClassObject::Div(ObjectPtr obj) {
  return Caller("__div__", self_.lock(), obj);
}

ObjectPtr DeclClassObject::DivMod(ObjectPtr obj) {
  return Caller("__mod__", self_.lock(), obj);
}

ObjectPtr DeclClassObject::RightShift(ObjectPtr obj) {
  return Caller("__rshift__", self_.lock(), obj);
}

ObjectPtr DeclClassObject::LeftShift(ObjectPtr obj) {
  return Caller("__lshift__", self_.lock(), obj);
}

ObjectPtr DeclClassObject::Lesser(ObjectPtr obj) {
  return Caller("__lt__", self_.lock(), obj);
}

ObjectPtr DeclClassObject::Greater(ObjectPtr obj) {
  return Caller("__gt__", self_.lock(), obj);
}

ObjectPtr DeclClassObject::LessEqual(ObjectPtr obj) {
  return Caller("__le__", self_.lock(), obj);
}

ObjectPtr DeclClassObject::GreatEqual(ObjectPtr obj) {
  return Caller("__ge__", self_.lock(), obj);
}

ObjectPtr DeclClassObject::Equal(ObjectPtr obj) {
  return Caller("__eq__", self_.lock(), obj);
}

ObjectPtr DeclClassObject::In(ObjectPtr obj) {
  return Caller("__contains__", self_.lock(), obj);
}

ObjectPtr DeclClassObject::NotEqual(ObjectPtr obj) {
  return Caller("__ne__", self_.lock(), obj);
}

ObjectPtr DeclClassObject::BitAnd(ObjectPtr obj) {
  return Caller("__rand__", self_.lock(), obj);
}

ObjectPtr DeclClassObject::BitOr(ObjectPtr obj) {
  return Caller("__ror__", self_.lock(), obj);
}

ObjectPtr DeclClassObject::BitXor(ObjectPtr obj) {
  return Caller("__rxor__", self_.lock(), obj);
}

ObjectPtr DeclClassObject::BitNot() {
  return Caller("__rinvert__", self_.lock());
}

ObjectPtr DeclClassObject::And(ObjectPtr obj) {
  return Caller("__and__", self_.lock(), obj);
}

ObjectPtr DeclClassObject::Or(ObjectPtr obj) {
  return Caller("__or__", self_.lock(), obj);
}

ObjectPtr DeclClassObject::GetItem(ObjectPtr obj) {
  return Caller("__getitem__", self_.lock(), obj);
}

ObjectPtr DeclClassObject::ObjIter(ObjectPtr /*obj*/) {
  return Caller("__iter__", self_.lock());
}

void DeclClassObject::DelItem(ObjectPtr /*obj*/) {
  Caller("__del__", self_.lock());
}

ObjectPtr DeclClassObject::UnaryAdd() {
  return Caller("__pos__", self_.lock());
}

ObjectPtr DeclClassObject::UnarySub() {
  return Caller("__neg__", self_.lock());
}

ObjectPtr DeclClassObject::Not() {
  return Caller("__invert__", self_.lock());
}

ObjectPtr DeclClassObject::Begin() {
  return Caller("__begin__", self_.lock());
}

ObjectPtr DeclClassObject::End() {
  return Caller("__end__", self_.lock());
}

ObjectPtr DeclClassObject::Next() {
  return Caller("__next__", self_.lock());
}

ObjectPtr DeclClassObject::HasNext() {
  return Caller("__has_next__", self_.lock());
}

ObjectPtr DeclClassObject::Call(Executor*, std::vector<ObjectPtr>&& params) {
  return Caller("__call__", std::move(params));
}

std::string DeclClassObject::Print() {
  ObjectPtr obj = Caller("__print__", self_.lock());

  if (obj->type() != ObjectType::STRING) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                      boost::format("print func must return string"));
  }

  return static_cast<StringObject&>(*obj).value();
}

long int DeclClassObject::Len() {
  ObjectPtr obj = Caller("__len__", self_.lock());

  if (obj->type() != ObjectType::INT) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                      boost::format("__len__ func must return integer"));
  }

  return static_cast<IntObject&>(*obj).value();
}

std::size_t DeclClassObject::Hash() {
  ObjectPtr obj = Caller("__hash__", self_.lock());

  if (obj->type() != ObjectType::INT) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                      boost::format("__hash__ func must return integer"));
  }

  return static_cast<std::size_t>(static_cast<IntObject&>(*obj).value());
}

ObjectPtr DeclClassObject::ObjBool() {
  return Caller("__bool__", self_.lock());
}

ObjectPtr DeclClassObject::ObjCmd() {
  return Caller("__cmd__", self_.lock());
}

ObjectPtr DeclClassObject::ObjString() {
  return Caller("__str__", self_.lock());
}

ObjectPtr DeclClassObject::Caller(const std::string& fname,
                                  std::vector<ObjectPtr>&& params) {
  SymbolTableStack& st =
      static_cast<DeclClassType&>(*ObjType()).SymTableStack();
  ObjectPtr func_obj = st.Lookup(fname, false).SharedAccess();

  if (func_obj->type() != ObjectType::FUNC) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                      boost::format("symbol %1% must be func")%fname);
  }

  params.insert(params.begin(), self_.lock());

  return static_cast<FuncObject&>(*func_obj).Call(nullptr, std::move(params));
}

}
}
