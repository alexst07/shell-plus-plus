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

#include "std-cmds.h"

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <iostream>
#include <cstdlib>

#include "objects/obj-type.h"

namespace seti {
namespace internal {
namespace cmds {
namespace stdf {

void CdCmd::Exec(Executor* /*parent*/, std::vector<std::string>&& args) {
  int stdout, stderr, stdin;

  std::tie(stdout, stderr, stdin) = GetStdFd();

  if (args.size() < 2) {
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;

    if (chdir(homedir) < 0) {
      std::stringstream ss;
      ss << "cd: " << homedir << ": " << strerror(errno) << "\n";
      write(stderr, ss.str().c_str(), ss.str().length());
      SetStatus(-1);
      return;
    }

    SetStatus(0);
    return;
  }

  if (chdir(args[1].c_str()) < 0) {
    std::stringstream ss;
    ss << "cd: " << args[1] << ": " << strerror(errno) << "\n";
    write(stderr, ss.str().c_str(), ss.str().length());
    SetStatus(-1);
    return;
  }

  SetStatus(0);
  return;
}

void ExitCmd::Exec(Executor* /*parent*/, std::vector<std::string>&& args) {
  if (args.size() < 2) {
    std::exit(0);
  }

  int status = 0;

  try {
    status = std::stoi(args[1]);
  } catch (std::invalid_argument&) {
    std::exit(-1);
  }

  std::exit(status);
}

}
}
}
}
