#include <iostream>
#include <fstream>
#include <sstream>

#include "parser/parser.h"
#include "ast/ast-printer.h"

int main(int argc, char **argv) {
  using namespace setti::internal;

  if (argc < 2) {
    std::cout << "usage: print_ast <file>\n";
    return -1;
  }

  std::ifstream file(argv[1]);
  std::stringstream buffer;
  buffer << file.rdbuf();

  Lexer l(buffer.str());

  TokenStream ts = l.Scanner();
  std::cout << "Scanner:\n";
  do {
    Token t = ts.CurrentToken();
    std::cout << t;
  } while (ts.Advance());
}

