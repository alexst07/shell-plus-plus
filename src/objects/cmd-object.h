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

#ifndef SHPP_CMD_OBJECT_H
#define SHPP_CMD_OBJECT_H

#include <boost/algorithm/string.hpp>
#include <iostream>
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

class CmdIterObject : public BaseIter {
 public:
  CmdIterObject(std::string delim, int outerr, ObjectPtr cmd_obj,
                ObjectPtr obj_type, SymbolTableStack&& sym_table);

  virtual ~CmdIterObject() {}

  ObjectPtr Next() override;

  ObjectPtr HasNext() override;

  std::string Print() override { return std::string("[cmd_iter]"); }

 private:
  size_t pos_;
  ObjectPtr cmd_obj_;
  std::vector<std::string> str_split_;
};

class CmdObject : public Object {
 public:
  CmdObject(int status, std::string&& str_stdout, std::string&& str_stderr,
            ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::CMD, obj_type, std::move(sym_table)),
        status_(status),
        str_stdout_(std::move(str_stdout)),
        str_stderr_(std::move(str_stderr)),
        delim_("\n") {
    boost::trim(str_stdout_);
    boost::trim(str_stderr_);
  }

  virtual ~CmdObject() {}

  ObjectPtr ObjIter(ObjectPtr obj) override;

  ObjectPtr ObjArray() override;

  ObjectPtr In(ObjectPtr obj) override;

  const std::string& str_stdout() const noexcept { return str_stdout_; }

  const std::string& str_stderr() const noexcept { return str_stderr_; }

  ObjectPtr ObjString() override;

  ObjectPtr ObjCmd() override;

  ObjectPtr ObjBool() override;

  ObjectPtr Not() override;

  bool Compare(ObjectPtr obj);

  ObjectPtr Equal(ObjectPtr obj) override;

  ObjectPtr NotEqual(ObjectPtr obj) override;

  std::shared_ptr<Object> Attr(std::shared_ptr<Object> self,
                               const std::string& name) override;

  std::string Print() override { return str_stdout_; }

  long int Len() override { return str_stdout_.size(); }

  inline void set_delim(const std::string& delim) { delim_ = delim; }

  const std::string delim() { return delim_; }

  int status() const { return status_; }

 private:
  int status_;
  std::string str_stdout_;
  std::string str_stderr_;
  std::string delim_;
};

class CmdType : public TypeObject {
 public:
  CmdType(ObjectPtr obj_type, SymbolTableStack&& sym_table);

  virtual ~CmdType() {}

  ObjectPtr Constructor(Executor*, Args&&, KWArgs&&) override;
};

class CmdOutFunc : public FuncObject {
 public:
  CmdOutFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class CmdErrFunc : public FuncObject {
 public:
  CmdErrFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class CmdDelimFunc : public FuncObject {
 public:
  CmdDelimFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

class CmdStatusFunc : public FuncObject {
 public:
  CmdStatusFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);
};

}  // namespace internal
}  // namespace shpp

#endif  // SHPP_CMD_OBJECT_H
