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

#include "interpreter.h"

#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>

#include "stmt-executor.h"
#include "executor.h"
#include "scope-executor.h"
#include "parser/parser.h"
#include "parser/lexer.h"
#include "modules/std-funcs.h"
#include "modules/path.h"
#include "objects/object-factory.h"
#include "modules/std-cmds.h"

namespace seti {
namespace internal {

Interpreter::Interpreter(bool main)
    : symbol_table_(SymbolTablePtr(new SymbolTable))
    , symbol_table_stack_(symbol_table_)
    , main_(main) {
  AlocTypes(symbol_table_stack_);

  module::stdf::RegisterModule(symbol_table_stack_);
  module::path::RegisterModule(symbol_table_stack_);
  cmds::stdf::RegisterCmds(symbol_table_stack_);

  RegisterVars();
}

Interpreter::~Interpreter() {
  // avoid errors of memory leak using sanytise flag
  symbol_table_.~shared_ptr();
}

void Interpreter::InsertVar(const std::string& name, ObjectPtr obj) {
  SymbolAttr symbol(obj, true);
  symbol_table_stack_.InsertEntry(name, std::move(symbol));
}

void Interpreter::RegisterVars() {
  ObjectFactory obj_factory(symbol_table_stack_);

  InsertVar("__main__", obj_factory.NewBool(main_));
}

void Interpreter::RegisterFileVars(const std::string& file) {
  namespace fs = boost::filesystem;

  ObjectFactory obj_factory(symbol_table_stack_);

  fs::path full_path = fs::system_complete(file);
  InsertVar("__file_path__", obj_factory.NewString(full_path.string()));

  fs::path file_name = full_path.filename();
  InsertVar("__file__", obj_factory.NewString(file_name.string()));

  fs::path parent_path = full_path.parent_path();
  InsertVar("__path__", obj_factory.NewString(parent_path.string()));
}

void Interpreter::Exec(std::string name) {
  std::ifstream file(name);

  if (!file.is_open()) {
    throw RunTimeError(RunTimeError::ErrorCode::INTERPRETER_FILE,
                       boost::format("can't open file: %1%")%name,
                       Position{0, 0});
  }

  std::stringstream buffer;
  buffer << file.rdbuf();

  Lexer l(buffer.str());
  TokenStream ts = l.Scanner();
  Parser p(std::move(ts));
  auto res = p.AstGen();
  stmt_list_ = res.MoveAstNode();

  if (p.nerrors() == 0) {
    RegisterFileVars(name);
    RootExecutor executor(symbol_table_stack_);
    executor.Exec(stmt_list_.get());
  } else {
    Message msg = p.Msgs();
    throw RunTimeError(RunTimeError::ErrorCode::PARSER, msg.msg(),
                       Position{msg.line(), msg.pos()});
  }
}

void Interpreter::ExecInterative(
    const std::function<std::string(bool concat)>& func) {
  RootExecutor executor(symbol_table_stack_);
  bool concat = false;
  std::string str_source;

  while (true) {
    std::string line = func(concat);
    if (concat) {
      str_source += std::string("\n") +  line;
    } else {
      str_source = line;
    }

    if (str_source.empty()) {
      continue;
    }

    Lexer l(str_source);
    TokenStream ts = l.Scanner();
    Parser p(std::move(ts));
    auto res = p.AstGen();
    std::unique_ptr<StatementList> stmt_list = res.MoveAstNode();

    if (p.nerrors() == 0) {
      concat = false;
      executor.Exec(stmt_list.get());
    } else {
      if (p.StmtIncomplete()) {
        concat = true;
        continue;
      } else {
        concat = false;
        Message msg = p.Msgs();
        throw RunTimeError(RunTimeError::ErrorCode::PARSER, msg.msg(),
                           Position{msg.line(), msg.pos()});
      }
    }
  }
}

}
}
