#include "parser.h"

#include <sstream>

namespace setti {
namespace internal {

ParserResult<Declaration> Parser::ParserMethodDeclaration() {
  // Advance func keyword
  Advance();
  ValidToken();

  std::unique_ptr<Identifier> id;

  if (token_ != TokenKind::IDENTIFIER) {
    ErrorMsg(boost::format("expected identifier"));
    return ParserResult<Declaration>(); // Error
  }

  id = std::move(factory_.NewIdentifier(boost::get<std::string>(
      token_.GetValue()), std::move(nullptr)));

  Advance();
  ValidToken();

  if (token_ != TokenKind::LPAREN) {
    ErrorMsg(boost::format("expected token '(' got %1%")% TokenValueStr());
    return ParserResult<Declaration>(); // Error
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
      return ParserResult<Declaration>(); // Error
    }

    // check params lists
    if (!ok) {
      return ParserResult<Declaration>(); // Error
    }

    Advance();
    ValidToken();
  }

  // abastract method
  if (token_ != TokenKind::LBRACE) {
    return ParserResult<Declaration>(factory_.NewFunctionDeclaration(
        std::move(func_params), std::move(id),
        std::unique_ptr<Block>(nullptr)));
  }

  std::unique_ptr<Block> block(ParserBlock().MoveAstNode<Block>());

  return ParserResult<Declaration>(factory_.NewFunctionDeclaration(
      std::move(func_params), std::move(id), std::move(block)));
}

ParserResult<ClassBlock> Parser::ParserClassBlock() {
  // advance lbrace
  Advance();
  ValidToken();

  std::vector<std::unique_ptr<Declaration>> decl_list;

  while (token_.IsNot(TokenKind::EOS, TokenKind::RBRACE)) {
    ValidToken();

    switch (token_) {
      case TokenKind::KW_FUNC: {
        ParserResult<Declaration> func(ParserMethodDeclaration());
        decl_list.push_back(func.MoveAstNode());
      } break;

      default:
        ErrorMsg(boost::format("function expected"));
        return ParserResult<Statement>(); // Error
    }
  }

  std::unique_ptr<NewClassDeclList> decl_list(factory_.NewClassDeclList(
      std::move(decl_list)));

  ParserResult<ClassBlock> class_block(factory_.NewClassBlock(
      std::move(decl_list)));

  return class_block;
}

std::vector<std::unique_ptr<Identifier>> Parser::ParserInterfaceList() {
  // advance colon
  Advance();
  ValidToken();

  if (token_ != TokenKind::IDENTIFIER) {
    ErrorMsg(boost::format("expected identifier"));
    return ParserResult<Declaration>(); // Error
  }

  std::vector<std::unique_ptr<Identifier>> id_list;

  do {
    ParserResult<Expression> ns_id(ParserScopeIdentifier);
    id_list.push_back(ns_id.MoveAstNode<Identifier>());
    ValidToken();
  } while (token_ != TokenKind::COMMA);

  return id_list;
}

ParserResult<ClassDeclaration> Parser::ParserClassDecl() {
  // advance class keyword
  Advance();
  ValidToken();

  std::unique_ptr<Identifier> class_name;
  std::unique_ptr<Identifier> parent;

  if (token_ != TokenKind::IDENTIFIER) {
    ErrorMsg(boost::format("expected identifier"));
    return ParserResult<Declaration>(); // Error
  }

  class_name = std::move(factory_.NewIdentifier(boost::get<std::string>(
      token_.GetValue()), std::move(nullptr)));

  Advance();
  ValidToken();

  // if lparen the class has a parent class
  // if not lparen and COLON the class implements interface
  if (token_ == TokenKind::LPAREN) {
    if (token_ != TokenKind::IDENTIFIER) {
      ErrorMsg(boost::format("expected identifier"));
      return ParserResult<Declaration>(); // Error
    }

    parent = std::move(factory_.NewIdentifier(boost::get<std::string>(
        token_.GetValue()), std::move(nullptr)));

    Advance();
    ValidToken();

    if (token_ != TokenKind::RPAREN) {
      ErrorMsg(boost::format("expected token )"));
      return ParserResult<Declaration>(); // Error
    }

    Advance();
    ValidToken();
  }

  std::vector<std::unique_ptr<Identifier>> interfaces;

  if (token_ == TokenKind::COLON) {
    interfaces = std::move(ParserInterfaceList());
  }

  if (token_ != TokenKind::LBRACE) {
    ErrorMsg(boost::format("expected token {"));
    return ParserResult<Declaration>(); // Error
  }

  ParserResult<ClassBlock> class_block(ParserClassBlock());

  // TODO: implement final keyword for class
  ParserResult<ClassDeclaration> class_decl(factory_.NewClassDeclaration(
      std::move(class_name), std::move(parent), std::move(interfaces),
      std::move(class_block), false));

  return class_decl;
}

}
}
