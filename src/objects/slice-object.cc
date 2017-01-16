#include "slice-object.h"

#include "simple-object.h"

namespace setti {
namespace internal {

SliceObject::SliceObject(ObjectPtr obj_start, ObjectPtr obj_end,
                         ObjectPtr obj_step, ObjectPtr obj_type,
                         SymbolTableStack&& sym_table)
    : Object(ObjectType::SLICE, obj_type, std::move(sym_table)) {
  if (obj_start->type() == ObjectType::INT) {
    IntObject& int_start = static_cast<IntObject&>(*obj_start);
    start_ = int_start.value();
    has_start_ = true;
  } else if (obj_start->type() == ObjectType::NIL) {
    start_ = 0;
    has_start_ = false;
  } else {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("start parameter must be integer"));
  }

  if (obj_end->type() == ObjectType::INT) {
    IntObject& int_end = static_cast<IntObject&>(*obj_end);
    end_ = int_end.value();
    has_end_ = true;
  } else if (obj_end->type() == ObjectType::NIL) {
    end_ = 0;
    has_end_ = false;
  } else {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("end parameter must be integer"));
  }

  if (obj_step->type() == ObjectType::INT) {
    IntObject& int_step = static_cast<IntObject&>(*obj_step);
    step_ = int_step.value();
    has_step_ = true;
  } else if (obj_step->type() == ObjectType::NIL) {
    step_ = 0;
    has_step_ = false;
  } else {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("step parameter must be integer"));
  }
}

bool SliceObject::operator==(const Object& obj) const {
  if (obj.type() != ObjectType::SLICE) {
    return false;
  }

  const SliceObject& slice = static_cast<const SliceObject&>(obj);

  bool exp = (start_ == slice.start_) && (end_ == slice.end_) &&
      (step_ == slice.step_);

  return exp;
}

std::tuple<int, int, int> SliceLogic(const SliceObject& slice, int size) {
  int start = 0;
  int end = size;
  int step = 1;

  if (slice.has_start()) {
    start = slice.start();
  }

  if (slice.has_end()) {
    end = slice.end();
  }

  if (slice.has_step()) {
    step = slice.step();
  }

  if (end < 0) {
    end = size - abs(end);
  }

  if (start < 0) {
    start = 0;
  }

  if (start > size) {
    start = size;
  }

  if (end > size) {
    end = size;
  }

  return std::tuple<int, int, int>(start, end, step);
}

}
}
