#ifndef SETI_TUPLE_OBJECT_H
#define SETI_TUPLE_OBJECT_H

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

class TupleObject: public Object {
 public:
   TupleObject(std::vector<std::unique_ptr<Object>>&& value,
               ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::TUPLE, obj_type, std::move(sym_table)),
        value_(value.size()) {
     for (size_t i = 0; i < value.size(); i++) {
       Object* obj_ptr = value[i].release();
       value_[i] = std::shared_ptr<Object>(obj_ptr);
     }
   }

   TupleObject(std::vector<std::shared_ptr<Object>>&& value, ObjectPtr obj_type,
               SymbolTableStack&& sym_table)
      : Object(ObjectType::TUPLE, obj_type, std::move(sym_table))
      , value_(std::move(value)) {}

   TupleObject(const TupleObject& obj): Object(obj), value_(obj.value_) {}

   virtual ~TupleObject() {}

   inline std::shared_ptr<Object>& ElementRef(size_t i) {
     return value_.at(i);
   }

   inline std::shared_ptr<Object> Element(size_t i) {
     return value_.at(i);
   }

   inline size_t Size() const noexcept {
     return value_.size();
   }

   inline void set(size_t i, std::unique_ptr<Object> obj) {
     Object* obj_ptr = obj.release();
     value_[i] = std::shared_ptr<Object>(obj_ptr);
   }

   ObjectPtr Element(const SliceObject& slice);

   ObjectPtr GetItem(ObjectPtr index) override;

   ObjectPtr& GetItemRef(ObjectPtr index) override;

   std::size_t Hash() const override;

   bool operator==(const Object& obj) const override;

   std::string Print() override {
     std::string str = "(";
     for (const auto& e: value_) {
       str += e->Print();
       str += ", ";
     }

     str = str.substr(0, str.length() - 2);
     str += ")";

     return str;
   }

 private:
  std::vector<std::shared_ptr<Object>> value_;
};

}
}

#endif  // SETI_TUPLE_OBJECT_H
