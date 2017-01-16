#include <iostream>
#include <fstream>
#include <sstream>

#include "interpreter/intepreter.h"

int main(int argc, char **argv) {
  using namespace setti::internal;

  if (argc < 2) {
    std::cout << "usage: interpreter <file>\n";
    return -1;
  }

  std::string name = argv[1];

  Interpreter i;
  try {
    i.Exec(name);
  } catch (setti::RunTimeError& e) {
    std::cout << "Error: " << e.pos().line << ": " << e.pos().col
              << ": " << e.what() << "\n";
  }
}
