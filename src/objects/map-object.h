#ifndef SETI_MAP_OBJECT_H
#define SETI_MAP_OBJECT_H

#include <memory>
#include <iostream>
#include <vector>

#include "run_time_error.h"
#include "ast/ast.h"
#include "interpreter/symbol-table.h"
#include "abstract-obj.h"

namespace setti {
namespace internal {

class MapObject: public Object {
 public:
  using Map =
      std::unordered_map<size_t, std::vector<std::pair<ObjectPtr, ObjectPtr>>>;

  using Pair = std::pair<size_t, std::vector<std::pair<ObjectPtr, ObjectPtr>>>;

  MapObject(std::vector<std::pair<ObjectPtr, ObjectPtr>>&& value,
            ObjectPtr obj_type, SymbolTableStack&& sym_table);

  MapObject(Map&& value, ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::MAP, obj_type, std::move(sym_table))
      , value_(std::move(value)) {}

  std::size_t Hash() const override {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("map object has no hash method"));
  }

  bool operator==(const Object& obj) const override;

  ObjectPtr GetItem(ObjectPtr index) override;

  ObjectPtr& GetItemRef(ObjectPtr index) override;

  // Return the reference for an object on the map, if there is no
  // entry for this index, create a new empty with this entry and
  // return its reference
  ObjectPtr& ElementRef(ObjectPtr obj_index);

  // Return a tuple object with the element and a bool object
  std::shared_ptr<Object> Element(ObjectPtr obj_index);

  // Create, this method doesn't do any kind of verification
  // the caller method must check if the entry exists on map or not
  ObjectPtr& Insert_(ObjectPtr obj_index);

  bool Exists(ObjectPtr obj_index);

  void Print() override {
    std::cout << "MAP: { ";
    for (auto& list: value_) {
      for (auto& pair: list.second) {
        std::cout << "(";
        pair.first->Print();
        std::cout << ", ";
        pair.second->Print();
        std::cout << ")";
      }
    }
    std::cout << "} ";
  }

 private:
   Map value_;
};

}
}

#endif  // SETI_MAP_OBJECT_H
