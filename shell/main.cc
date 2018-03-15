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

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include "runner.h"

void help() {
  std::cerr << "shpp [file]\n";
}

std::vector<std::string> Args(int argc, char **argv) {
  std::vector<std::string> args;

  if (argc > 1) {
    for (int i = 1; i < argc; i++) {
      args.push_back(std::string(argv[i]));
    }
  }

  return args;
}

int main(int argc, char **argv) {
  shpp::Runner runner;

  if (argc == 1) {
    runner.ExecInterative();
  } else {
    std::vector<std::string> args = Args(argc, argv);
    runner.Exec(argv[1], std::move(args));
  }

  return 0;
}
