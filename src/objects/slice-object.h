#ifndef SETI_SLICE_OBJECT_H
#define SETI_SLICE_OBJECT_H

#include <memory>
#include <iostream>
#include <vector>
#include <tuple>

#include "run_time_error.h"
#include "ast/ast.h"
#include "interpreter/symbol-table.h"
#include "abstract-obj.h"

namespace setti {
namespace internal {

class SliceObject: public Object {
 public:
  SliceObject(ObjectPtr obj_start, ObjectPtr obj_end, ObjectPtr obj_step,
              ObjectPtr obj_type, SymbolTableStack&& sym_table);

  SliceObject(const SliceObject& obj)
      : Object(obj), start_(obj.start_), end_(obj.end_), step_(obj.step_) {}

  virtual ~SliceObject() {}

  SliceObject& operator=(const SliceObject& obj) {
    start_ = obj.start_;
    end_ = obj.end_;
    step_ = obj.step_;
    return *this;
  }

  inline int start() const noexcept { return start_; }

  inline int end() const noexcept { return end_; }

  inline int step() const noexcept { return step_; }

  inline bool has_start() const noexcept { return has_start_; }

  inline bool has_end() const noexcept { return has_end_; }

  inline bool has_step() const noexcept { return has_step_; }

  std::size_t Hash() const override {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("slice object has no hash method"));
  }

  bool operator==(const Object& obj) const override;

  std::string Print() override {
    return std::string("[slice ") + std::to_string(start_ ) + ", " +
        std::to_string(end_);
  }

 private:
  int start_;
  int end_;
  int step_;

  bool has_start_;
  bool has_end_;
  bool has_step_;
};

// calculates the logic between start end and step for string, array and tuple
// the logic is the same fo all object, this function helps keep the consistence
std::tuple<int, int, int> SliceLogic(const SliceObject& slice, int size);

}
}

#endif  // SETI_SLICE_OBJECT_H
