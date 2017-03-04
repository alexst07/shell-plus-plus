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

#include "runner.h"

void help() {
  std::cerr << "shpp [file]\n";
}

int main(int argc, char **argv) {
  shpp::Runner runner;

  if (argc == 1) {
    runner.ExecInterative();
  } else if (argc == 2) {
    runner.Exec(argv[1]);
  } else {
    help();
    return -1;
  }

  return 0;
}
