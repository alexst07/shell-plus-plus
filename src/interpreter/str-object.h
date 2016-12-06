#ifndef SETI_STR_OBJECT_H
#define SETI_STR_OBJECT_H

#include <memory>
#include <iostream>

#include "run_time_error.h"
#include "ast/ast.h"
#include "symbol_table.h"
#include "obj_type.h"
#include "func_object.h"

namespace setti {
namespace internal {

class StringObject: public Object {
 public:
  StringObject(std::string&& value, ObjectPtr obj_type,
               SymbolTableStack&& sym_table)
      : Object(ObjectType::STRING, obj_type, std::move(sym_table))
      , value_(std::move(value)) {}

  StringObject(const std::string& value, ObjectPtr obj_type,
               SymbolTableStack&& sym_table)
      : Object(ObjectType::STRING, obj_type, std::move(sym_table))
      , value_(value) {}

  StringObject(const StringObject& obj): Object(obj), value_(obj.value_) {}

  virtual ~StringObject() {}

  StringObject& operator=(const StringObject& obj) {
    value_ = obj.value_;
    return *this;
  }

  inline const std::string& value() const noexcept { return value_; }

  std::size_t Hash() const override {
    std::hash<std::string> str_hash;
    return str_hash(value_);
  }

  bool operator==(const Object& obj) const override {
    if (obj.type() != ObjectType::STRING) {
      return false;
    }

    std::string value = static_cast<const StringObject&>(obj).value_;

    return value_ == value;
  }

  ObjectPtr Equal(ObjectPtr obj) override;

  ObjectPtr NotEqual(ObjectPtr obj) override;

  ObjectPtr Add(ObjectPtr obj) override;

  ObjectPtr Copy() override;

  std::shared_ptr<Object> Arrow(std::shared_ptr<Object> self,
                                const std::string& name) override;

  void Print() override {
    std::cout << "STRING: " << value_;
  }

 private:
  std::string value_;
};

class StringGetterFunc: public FuncObject {
 public:
  StringGetterFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);
};

}
}

#endif  // SETI_STR_OBJECT_H
