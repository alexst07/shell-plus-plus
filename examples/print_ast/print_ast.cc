#include <iostream>
#include <fstream>
#include <sstream>

#include "parser/parser.h"
#include "ast/ast-printer.h"

int main(int argc, char **argv) {
  using namespace seti::internal;

  if (argc < 2) {
    std::cout << "usage: print_ast <file>\n";
    return -1;
  }

  std::ifstream file(argv[1]);
  std::stringstream buffer;
  buffer << file.rdbuf();

  Lexer l(buffer.str());
  std::cout << "Lexer\n";
  TokenStream ts = l.Scanner();
  Parser p(std::move(ts));
  auto res = p.AstGen();

  if (p.nerrors() == 0) {
    std::cout << "Correct analysis\n";
    AstPrinter visitor;
    visitor.Visit(res.NodePtr());
  } else {
    std::cout << "Parser error analysis:\n";
    auto msgs = p.Msgs();
    for (const auto& msg : msgs) {
      std::cout << msg << "\n";
    }
  }
}
