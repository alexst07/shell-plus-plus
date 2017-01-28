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

#include "runner.h"

#include <cstdlib>

#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <readline/readline.h>

#include "env-shell.h"
#include "objects/str-object.h"

namespace seti {

Runner::Runner() {
  using namespace internal;

  internal::EnvShell::instance()->InitShell();
}

void Runner::Exec(std::string name) {
  try {
    interpreter_.Exec(name);
  } catch (RunTimeError& e) {
    std::cout << "Error: " << e.pos().line << ": " << e.pos().col
              << ": " << e.what() << "\n\n";

    for (auto& msg: e.messages()) {
      std::cout << "Error: " << msg.line() << ": " << msg.pos()
                << ": " << msg.msg() << "\n";
    }
  }
}

void Runner::ExecInterative() {
  while (true) {
    try {
      interpreter_.ExecInterative([&](internal::Executor* exec, bool concat) {
        char *input;
        std::string str_source;
        std::string prompt;

        if (concat) {
          prompt = "| ";

          internal::ObjectPtr obj_func = interpreter_.LookupSymbol("PS2");

          if (obj_func) {
            std::vector<internal::ObjectPtr> params;
            internal::ObjectPtr obj = obj_func->Call(exec, std::move(params));

            if (obj->type() == internal::Object::ObjectType::STRING) {
              prompt = static_cast<internal::StringObject&>(*obj).value();
            }
          }
        } else {
          prompt = "> ";

          internal::ObjectPtr obj_func = interpreter_.LookupSymbol("PS1");

          if (obj_func) {
            std::vector<internal::ObjectPtr> params;
            internal::ObjectPtr obj = obj_func->Call(exec, std::move(params));

            if (obj->type() == internal::Object::ObjectType::STRING) {
              prompt = static_cast<internal::StringObject&>(*obj).value();
            }
          }
        }

        input = readline(prompt.c_str());

        if (input == nullptr) {
          exit(0);
        }

        str_source = input;
        free(input);
        return str_source;
      });
    } catch (RunTimeError& e) {
      std::cout << "Error: " << e.pos().line << ": " << e.pos().col
                << ": " << e.what() << "\n\n";

      for (auto& msg: e.messages()) {
        std::cout << "Error: " << msg.line() << ": " << msg.pos()
                  << ": " << msg.msg() << "\n";
      }
    }
  }
}

}
