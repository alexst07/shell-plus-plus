#ifndef SETI_SLICE_OBJECT_H
#define SETI_SLICE_OBJECT_H

#include <memory>
#include <iostream>
#include <vector>

#include "run_time_error.h"
#include "ast/ast.h"
#include "interpreter/symbol-table.h"
#include "abstract-obj.h"

namespace setti {
namespace internal {

class SliceObject: public Object {
 public:
  SliceObject(ObjectPtr obj_start, ObjectPtr obj_end, ObjectPtr obj_type,
              SymbolTableStack&& sym_table)
      : Object(ObjectType::SLICE, obj_type, std::move(sym_table)) {
    if (obj_start->type() != ObjectType::INT ||
        obj_end->type() != ObjectType::INT) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("slice parameter must be integer"));

      IntObject& int_start = static_cast<IntObject&>(*obj_start);
      IntObject& int_end = static_cast<IntObject&>(*obj_end);

      start_ = int_start.value();
      end_ = int_end.value();
    }
  }

  SliceObject(const SliceObject& obj)
      : Object(obj), start_(obj.start_), end_(obj.end_) {}

  virtual ~SliceObject() {}

  SliceObject& operator=(const SliceObject& obj) {
    start_ = obj.start_;
    end_ = obj.end_;
    return *this;
  }

  inline int start() const noexcept { return start_; }
  inline int end() const noexcept { return start_; }

  std::size_t Hash() const override {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("slice object has no hash method"));
  }

  bool operator==(const Object& obj) const override {
    if (obj.type() != ObjectType::SLICE) {
      return false;
    }

    const SliceObject& slice = static_cast<const SliceObject&>(obj);

    bool exp = (start_ == slice.start_) && (end_ == slice.end_);

    return exp;
  }

  void Print() override {
    std::cout << "SLICE: start = " << start_ << ", end = " << end_;
  }

 private:
  int start_;
  int end_;
};

}
}

#endif  // SETI_SLICE_OBJECT_H
