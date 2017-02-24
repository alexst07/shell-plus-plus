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

#ifndef SETI_REGEX_OBJECT_H
#define SETI_REGEX_OBJECT_H

#include <memory>
#include <iostream>
#include <vector>
#include <regex>

#include "run_time_error.h"
#include "ast/ast.h"
#include "interpreter/symbol-table.h"
#include "abstract-obj.h"
#include "obj-type.h"
#include "func-object.h"

namespace seti {
namespace internal {

class RegexObject: public Object {
 public:
   RegexObject(const std::string& str_expr, ObjectPtr obj_type,
      SymbolTableStack&& sym_table);

   virtual ~RegexObject() {}

   ObjectPtr ObjString() override;

   ObjectPtr Attr(std::shared_ptr<Object> self,
                  const std::string& name) override;

   std::vector<std::vector<std::string>> Search(const std::string& str_search);

   bool Match(const std::string& str);

   std::string Print() override {
     return "regex<" + str_expr_ + ">";
   }

   long int Len() override {
     return str_expr_.size();
   }

 private:
  std::string str_expr_;
  std::regex re_;
};

class RegexType: public TypeObject {
 public:
  RegexType(ObjectPtr obj_type, SymbolTableStack&& sym_table);

  virtual ~RegexType() {}

  ObjectPtr Constructor(Executor*, std::vector<ObjectPtr>&& params) override;
};

class RegexMatchFunc: public FuncObject {
 public:
  RegexMatchFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);
};

class RegexSearchFunc: public FuncObject {
 public:
  RegexSearchFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);
};

}
}

#endif  // SETI_REGEX_OBJECT_H
