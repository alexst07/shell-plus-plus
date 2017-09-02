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

#ifndef SHPP_EXCEPTION_OBJECT_H
#define SHPP_EXCEPTION_OBJECT_H

#include <memory>
#include <iostream>
#include <vector>
#include <string>

#include "run_time_error.h"
#include "ast/ast.h"
#include "interpreter/symbol-table.h"
#include "abstract-obj.h"
#include "obj-type.h"
#include "func-object.h"

namespace shpp {
namespace internal {

#define DECLARE_EXCEPTION(NAME)                                               \
  class NAME ## Object: public Object {                                       \
  public:                                                                     \
    NAME ## Object(const std::string& msg, ObjectPtr obj_type,                \
        SymbolTableStack&& sym_table);                                        \
                                                                              \
    virtual ~NAME ## Object() {}                                              \
                                                                              \
    ObjectPtr ObjString() override;                                           \
                                                                              \
    ObjectPtr Attr(std::shared_ptr<Object> self,                              \
                    const std::string& name) override;                        \
                                                                              \
    std::string Print() override {                                            \
      return msg_;                                                            \
    }                                                                         \
                                                                              \
  private:                                                                    \
    std::string msg_;                                                         \
  };                                                                          \
                                                                              \
  class NAME ## Type: public TypeObject {                                     \
  public:                                                                     \
    NAME ## Type(ObjectPtr obj_type, SymbolTableStack&& sym_table,            \
        ObjectPtr base);                                                      \
                                                                              \
    virtual ~NAME ## Type() {}                                                \
                                                                              \
    std::shared_ptr<Object> Attr(std::shared_ptr<Object> self,                \
                               const std::string& name) override;             \
                                                                              \
    ObjectPtr Constructor(Executor*, Args&& params, KWArgs&&) override;       \
  };                                                                          \
                                                                              \
  class NAME ## InitFunc: public FuncObject {                                 \
  public:                                                                     \
    NAME ## InitFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)        \
        : FuncObject(obj_type, std::move(sym_table)) {}                       \
                                                                              \
    ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);            \
  };                                                                          \
                                                                              \
  class NAME ## StrFunc: public FuncObject {                                  \
  public:                                                                     \
    NAME ## StrFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)         \
        : FuncObject(obj_type, std::move(sym_table)) {}                       \
                                                                              \
    ObjectPtr Call(Executor* /*parent*/, Args&& params, KWArgs&&);            \
  };                                                                          \


DECLARE_EXCEPTION(Exception)
DECLARE_EXCEPTION(NullAccessException)
DECLARE_EXCEPTION(LookupException)
DECLARE_EXCEPTION(InvalidCmdException)
DECLARE_EXCEPTION(BadAllocException)
DECLARE_EXCEPTION(IndexException)
DECLARE_EXCEPTION(KeyException)
DECLARE_EXCEPTION(InvalidArgsException)
DECLARE_EXCEPTION(TypeException)
DECLARE_EXCEPTION(FuncParamsException)
DECLARE_EXCEPTION(ZeroDivException)
DECLARE_EXCEPTION(FdNotFoundException)
DECLARE_EXCEPTION(IOException)
DECLARE_EXCEPTION(ImportException)
DECLARE_EXCEPTION(AssertException)
DECLARE_EXCEPTION(ParserException)
DECLARE_EXCEPTION(RegexException)
DECLARE_EXCEPTION(GlobException)
DECLARE_EXCEPTION(EvalException)
DECLARE_EXCEPTION(ErrorException)

ObjectPtr MapExceptionError(RunTimeError& err, SymbolTableStack& sym_table);

}
}

#endif  // SHPP_EXCEPTION_OBJECT_H

