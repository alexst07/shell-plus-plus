#include "parser.h"

#include <sstream>

namespace setti {
namespace internal {

ParserResult<FunctionDeclaration> Parser::ParserMethodDeclaration() {
  // Advance func keyword
  Advance();
  ValidToken();

  std::unique_ptr<Identifier> id;

  if (token_ != TokenKind::IDENTIFIER) {
    ErrorMsg(boost::format("expected identifier"));
    return ParserResult<FunctionDeclaration>(); // Error
  }

  id = std::move(factory_.NewIdentifier(boost::get<std::string>(
      token_.GetValue()), std::move(nullptr)));

  Advance();
  ValidToken();

  if (token_ != TokenKind::LPAREN) {
    ErrorMsg(boost::format("expected token '(' got %1%")% TokenValueStr());
    return ParserResult<FunctionDeclaration>(); // Error
  }

  Advance();
  ValidToken();

  std::vector<std::unique_ptr<FunctionParam>> func_params;

  // parser the parameters list
  if (token_ == TokenKind::RPAREN) {
    Advance();
    ValidToken();
  } else {
    bool ok = true;
    std::tie(func_params, ok) = ParserParamsList();
    if (token_ != TokenKind::RPAREN) {
      ErrorMsg(boost::format("expected token ')'"));
      return ParserResult<FunctionDeclaration>(); // Error
    }

    // check params lists
    if (!ok) {
      return ParserResult<FunctionDeclaration>(); // Error
    }

    Advance();
    ValidToken();
  }

  // abastract method
  if (token_ != TokenKind::LBRACE) {
    return ParserResult<FunctionDeclaration>(factory_.NewFunctionDeclaration(
        std::move(func_params), std::move(id),
        std::unique_ptr<Block>(nullptr)));
  }

  std::unique_ptr<Block> block(ParserBlock().MoveAstNode<Block>());

  return ParserResult<FunctionDeclaration>(factory_.NewFunctionDeclaration(
      std::move(func_params), std::move(id), std::move(block)));
}

}
}
