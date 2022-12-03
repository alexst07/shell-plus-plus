#include <iostream>

#include "parser/parser_literal_string.h"
int main(int argc, char* argv[]) {
  std::cout << "String: " << argv[1] << std::endl;
  shpp::internal::ParserLiteralString parser(argv[1]);
  parser.Scanner();
  std::vector<shpp::internal::LiteralStringToken>& vec =
      parser.getStringTokens();

  std::cout << "Tokens count: " << vec.size() << std::endl;
  for (shpp::internal::LiteralStringToken& tk : vec) {
    std::cout << "String[" << tk.IsInterpretable() << "]:" << tk.GetStrToken()
              << std::endl;
  }
  return 0;
}