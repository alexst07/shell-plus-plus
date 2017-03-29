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

#ifndef SHPP_SLICE_OBJECT_H
#define SHPP_SLICE_OBJECT_H

#include <memory>
#include <iostream>
#include <vector>
#include <tuple>

#include "run_time_error.h"
#include "ast/ast.h"
#include "interpreter/symbol-table.h"
#include "abstract-obj.h"

namespace shpp {
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

  std::size_t Hash() override {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("slice object has no hash method"));
  }

  bool operator==(const Object& obj) override;

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

#endif  // SHPP_SLICE_OBJECT_H
