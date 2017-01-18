#include <iostream>
#include <fstream>
#include <sstream>

#include "runner.h"

void help() {
  std::cerr << "seti [file]\n";
}

int main(int argc, char **argv) {
  seti::Runner runner;

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
