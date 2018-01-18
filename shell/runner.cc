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
#include <functional>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include "env-shell.h"
#include "objects/str-object.h"
#include "utils/dir.h"

namespace shpp {

static sigjmp_buf ctrlc_buf;

void HandleSignals(int signo) {
  if (signo == SIGINT) {
    std::cout << "\n";
    siglongjmp(ctrlc_buf, 1);
  }
}

Runner::Runner(): interpreter_(true) {
  using namespace internal;

  internal::EnvShell::instance()->InitShell();
}

void Runner::Exec(std::string name, std::vector<std::string>&& args) {
  try {
    internal::ScriptStream file(name);

    if (!file.IsOpen()) {
      throw std::invalid_argument((boost::format("can't open file: %1%")%name)
          .str());
    }

    interpreter_.Exec(file, std::move(args));
  } catch (RunTimeError& e) {
    std::cout << "File: '" << e.file()  << "'"
              << "\n  line: " << e.pos().line
              << "  >> " << e.line_error() << "\n"
              << "Error: " << e.what() << "\n\n";

    for (auto& msg: e.messages()) {
      std::cout << "File: '" << msg.file()  << "'"
                << "\n  line: " << msg.line()
                << "  >> " << msg.line_error() << "\n"
                << "Error: " << msg.msg() << "\n\n";
    }
  } catch (std::invalid_argument& e) {
    std::cout << "Error: " << e.what() << "\n\n";
  }
}

InteractiveRunner::InteractiveRunner(): interpreter_(true), readline_(500) {
  using internal::EnvShell;
  using std::placeholders::_1;
  using std::placeholders::_2;

  EnvShell::instance()->InitShell();

  std::function<CompleteFnRet(const std::vector<std::string>&, bool)> fn_comp =
      std::bind(&InteractiveRunner::CompleteFn, this, _1, _2);

  std::function<readline::Text(const std::string& line)> fn_highlight =
      std::bind(&InteractiveRunner::HighlightFn, this, _1);

  readline_.SetCompleteFunc(std::move(fn_comp));
  readline_.SetHighlightFunc(std::move(fn_highlight));
}

void InteractiveRunner::ExecInterative() {
  namespace fs = boost::filesystem;
  using std::placeholders::_1;

  if (signal(SIGINT, HandleSignals) == SIG_ERR) {
    std::cerr << "failed to register interrupts with kernel\n";
    exit(-1);
  }

  internal::EnvShell::instance()->interective_exec(true);

  fs::path path_rc = fs::path(internal::GetHome() + "/.shpprc");

  if (fs::exists(path_rc)) {
    Exec(path_rc.string());
  }

  while (true) {
    try {
      interpreter_.ExecInterative([&](internal::Executor* exec, bool concat) {
        boost::optional<std::string> input;
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

        while (sigsetjmp(ctrlc_buf, 1) != 0 );

        readline::Text tprompt = MountText(prompt);
        input = readline_.Prompt(tprompt);

        if (!input) {
          exit(0);
        }

        str_source = *input;

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

void InteractiveRunner::Exec(std::string name,
    std::vector<std::string>&& args) {
  try {
    internal::ScriptStream file(name);

    if (!file.IsOpen()) {
      throw std::invalid_argument((boost::format("can't open file: %1%")%name)
          .str());
    }

    interpreter_.Exec(file, std::move(args));
  } catch (RunTimeError& e) {
    std::cout << "File: '" << e.file()  << "'"
              << "\n  line: " << e.pos().line
              << "  >> " << e.line_error() << "\n"
              << "Error: " << e.what() << "\n\n";

    for (auto& msg: e.messages()) {
      std::cout << "File: '" << msg.file()  << "'"
                << "\n  line: " << msg.line()
                << "  >> " << msg.line_error() << "\n"
                << "Error: " << msg.msg() << "\n\n";
    }
  } catch (std::invalid_argument& e) {
    std::cout << "Error: " << e.what() << "\n\n";
  }
}

readline::Text InteractiveRunner::MountText(const std::string& str) {
  bool in_style = false;

  readline::Text text;
  std::string text_piece = "";

  for (size_t i = 0; i < str.length(); i++) {
    if (str[i] == '\033') {
      in_style = true;
      text << text_piece;
      text_piece = "";
      text_piece += str[i];
    } else if (str[i] == 'm' && in_style) {
      in_style = false;
      text_piece += str[i];
      text << readline::Style(text_piece);
      text_piece = "";
    } else {
      text_piece += str[i];
    }
  }

  return text;
}

InteractiveRunner::CompleteFnRet InteractiveRunner::CompleteFn(
    const std::vector<std::string>& args, bool tip) {
  return readline::RetDirFileList(args, tip,
      readline::ListDirType::FILES_DIR);
}

readline::Text InteractiveRunner::HighlightFn(const std::string& line) {
  // format the first argument to set color white
  readline::Text text;

  // add the style on the text
  // all style must be add with Style class
  text << readline::Style("\e[34m");

  for (size_t i = 0; i < line.length(); i++) {
    if (line[i] == ' ') {
      text << readline::Style("\e[0m");
    }

    char c = line[i];
    std::string s;
    s += c;

    // text have the operator << overloaded to receive data like std::cout
    text << s;
  }

  return text;
}

}
