#ifndef SETI_ARRAY_OBJECT_H
#define SETI_ARRAY_OBJECT_H

#include <memory>
#include <iostream>
#include <vector>

#include "run_time_error.h"
#include "ast/ast.h"
#include "symbol_table.h"
#include "abstract-obj.h"

namespace setti {
namespace internal {

class ArrayIterObject: public Object {
 public:
  ArrayIterObject(ObjectPtr array_obj, ObjectPtr obj_type,
                  SymbolTableStack&& sym_table)
      : Object(ObjectType::ARRAY_ITER, obj_type, std::move(sym_table))
      , pos_(0) {
    if (array_obj->type() != ObjectType::ARRAY) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("invalid conversion to int"));
    }

    array_obj_ = array_obj;
  }

  virtual ~ArrayIterObject() {}

  ObjectPtr Equal(ObjectPtr obj) override;

  ObjectPtr Next() override;

  ObjectPtr HasNext() override;

  void Print() override {
    std::cout << "ARRAY ITER: ";
  }

 private:
  // it uses the array object and position insted of c++ iterator
  // because the iterator object has need a shared_reference
  // of object, because the array could be removed from memory
  // if the object was created inside a loop for example
  // and the iterator could be used outside this loop
  ObjectPtr array_obj_;
  size_t pos_;
};

class ArrayObject: public Object {
 public:
   ArrayObject(std::vector<std::unique_ptr<Object>>&& value,
               ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::ARRAY, obj_type, std::move(sym_table))
      , value_(value.size()) {
     for (size_t i = 0; i < value.size(); i++) {
       Object* obj_ptr = value[i].release();
       value_[i] = std::shared_ptr<Object>(obj_ptr);
     }
   }

   ArrayObject(std::vector<std::shared_ptr<Object>>&& value, ObjectPtr obj_type,
               SymbolTableStack&& sym_table)
      : Object(ObjectType::ARRAY, obj_type, std::move(sym_table))
      , value_(value) {}

   ArrayObject(const ArrayObject& obj): Object(obj), value_(obj.value_) {}

   virtual ~ArrayObject() {}

   inline Object* at(size_t i) {
     return value_.at(i).get();
   }

   inline std::shared_ptr<Object>& ElementRef(size_t i) {
     return value_.at(i);
   }

   inline std::shared_ptr<Object> Element(size_t i) {
     return value_.at(i);
   }

   inline void set(size_t i, std::unique_ptr<Object> obj) {
     Object* obj_ptr = obj.release();
     value_[i] = std::shared_ptr<Object>(obj_ptr);
   }

   ObjectPtr ObjIter(ObjectPtr obj) override;

   std::size_t Hash() const override;

   bool operator==(const Object& obj) const override;

   size_t ArraySize() const noexcept {
     return value_.size();
   }

   void Print() override {
     std::cout << "ARRAY: [ ";
     for (const auto& e: value_) {
       e->Print();
       std::cout << " ";
     }
     std::cout << "]";
   }

 private:
  std::vector<std::shared_ptr<Object>> value_;
};

}
}

#endif  // SETI_ARRAY_OBJECT_H
