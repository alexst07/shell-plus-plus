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

#ifndef SHPP_GLOB_OBJECT_H
#define SHPP_GLOB_OBJECT_H

#include <memory>
#include <vector>

#include "abstract-obj.h"
#include "ast/ast.h"
#include "func-object.h"
#include "interpreter/symbol-table.h"
#include "obj-type.h"
#include "run_time_error.h"

namespace shpp {
namespace internal {

class GlobIterObject : public BaseIter {
 public:
  GlobIterObject(ObjectPtr glob_obj, ObjectPtr obj_type,
                 SymbolTableStack&& sym_table);

  virtual ~GlobIterObject() {}

  ObjectPtr Next() override;

  ObjectPtr HasNext() override;

  std::string Print() override { return std::string("[glob_iter]"); }

 private:
  size_t pos_;
  ObjectPtr glob_obj_;
};

class GlobObject : public Object {
 public:
  GlobObject(const std::string& str_glob_expr, bool recursive,
             ObjectPtr obj_type, SymbolTableStack&& sym_table);

  virtual ~GlobObject() {}

  ObjectPtr ObjIter(ObjectPtr obj) override;

  ObjectPtr ObjArray() override;

  ObjectPtr In(ObjectPtr obj) override;

  ObjectPtr ObjBool() override;

  ObjectPtr Not() override;

  std::shared_ptr<Object> Attr(std::shared_ptr<Object> self,
                               const std::string& name) override;

  std::string Print() override;

  long int Len() override { return glob_result_vec_.size(); }

  ObjectPtr GetGlobItem(size_t pos) const noexcept {
    return glob_result_vec_[pos];
  }

  std::vector<ObjectPtr>& value() noexcept { return glob_result_vec_; }

  const std::vector<ObjectPtr>& value() const noexcept {
    return glob_result_vec_;
  }

 private:
  std::string str_glob_expr_;
  bool full_;
  std::vector<ObjectPtr> glob_result_vec_;
};

class GlobType : public TypeObject {
 public:
  GlobType(ObjectPtr obj_type, SymbolTableStack&& sym_table);

  virtual ObjectPtr Constructor(Executor* /*parent*/, Args&& params, KWArgs&&);

  virtual ~GlobType() {}
};

class GlobSearchFunc : public FuncObject {
 public:
  GlobSearchFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

}  // namespace internal
}  // namespace shpp

#endif