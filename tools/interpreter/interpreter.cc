#include <iostream>
#include <fstream>
#include <sstream>

#include "interpreter/interpreter.h"

int main(int argc, char **argv) {
  using namespace shpp::internal;

  if (argc < 2) {
    std::cout << "usage: interpreter <file>\n";
    return -1;
  }

  std::string name = argv[1];

  Interpreter i;
  try {
    ScriptStream file(name);
    i.Exec(file);
  } catch (shpp::RunTimeError& e) {
    std::cout << "Error: " << e.pos().line << ": " << e.pos().col
              << ": " << e.what() << "\n";
  }
}
