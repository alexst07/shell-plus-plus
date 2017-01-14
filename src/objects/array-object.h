#ifndef SETI_ARRAY_OBJECT_H
#define SETI_ARRAY_OBJECT_H

#include <memory>
#include <iostream>
#include <vector>

#include "run_time_error.h"
#include "ast/ast.h"
#include "interpreter/symbol-table.h"
#include "abstract-obj.h"
#include "slice-object.h"

namespace setti {
namespace internal {

class ArrayIterObject: public Object {
 public:
  ArrayIterObject(ObjectPtr array_obj, ObjectPtr obj_type,
                  SymbolTableStack&& sym_table);

  virtual ~ArrayIterObject() {}

  ObjectPtr Equal(ObjectPtr obj) override;

  ObjectPtr Next() override;

  ObjectPtr HasNext() override;

  std::string Print() override {
    return std::string("[array_iter]");
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
               ObjectPtr obj_type, SymbolTableStack&& sym_table);

   ArrayObject(std::vector<std::shared_ptr<Object>>&& value, ObjectPtr obj_type,
               SymbolTableStack&& sym_table);

   ArrayObject(const ArrayObject& obj);

   virtual ~ArrayObject() = default;

   inline Object* at(size_t i) {
     return value_.at(i).get();
   }

   inline std::shared_ptr<Object>& ElementRef(size_t i) {
     return value_.at(i);
   }

   inline std::shared_ptr<Object> Element(size_t i) {
     return value_.at(i);
   }

   ObjectPtr Element(const SliceObject& slice);

   ObjectPtr GetItem(ObjectPtr index) override;

   ObjectPtr& GetItemRef(ObjectPtr index) override;

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

   std::string Print() override;

 private:
  std::vector<std::shared_ptr<Object>> value_;
};

}
}

#endif  // SETI_ARRAY_OBJECT_H