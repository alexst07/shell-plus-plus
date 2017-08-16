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

#include "parser.h"

#include <sstream>

namespace shpp {
namespace internal {

ParserResult<Declaration> Parser::ParserMethodDeclaration() {
  // get the method position
  Position pos = {token_.Line(), token_.Col()};

  // Advance func keyword
  Advance();
  ValidToken();

  std::unique_ptr<Identifier> id;

  if (token_ != TokenKind::IDENTIFIER) {
    ErrorMsg(boost::format("expected identifier got %1%")% TokenValueStr());
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
        std::unique_ptr<Block>(nullptr), pos));
  }

  std::unique_ptr<Block> block(ParserBlock().MoveAstNode<Block>());

  return ParserResult<Declaration>(factory_.NewFunctionDeclaration(
      std::move(func_params), std::move(id), std::move(block), pos));
}

ParserResult<ClassBlock> Parser::ParserClassBlock() {
  // advance lbrace
  Advance();

  std::vector<std::unique_ptr<Declaration>> decl_list;

  while (ValidToken().IsNot(TokenKind::EOS, TokenKind::RBRACE)) {
    ValidToken();

    switch (token_.GetKind()) {
      case TokenKind::KW_FUNC: {
        ParserResult<Declaration> func(ParserMethodDeclaration());
        decl_list.push_back(func.MoveAstNode());
      } break;

      case TokenKind::KW_CLASS: {
        ParserResult<Declaration> class_decl(ParserClassDecl());
        decl_list.push_back(class_decl.MoveAstNode());
      } break;

      default:
        ErrorMsg(boost::format("declaration expected"));
        return ParserResult<ClassBlock>(); // Error
    }
  }

  if (ValidToken() != TokenKind::RBRACE) {
    ErrorMsg(boost::format("expected } token, got %1%")% TokenValueStr());
      return ParserResult<ClassBlock>(); // Error
  }

  Advance();
  ValidToken();

  std::unique_ptr<ClassDeclList> class_list(factory_.NewClassDeclList(
      std::move(decl_list)));

  ParserResult<ClassBlock> class_block(factory_.NewClassBlock(
      std::move(class_list)));

  return class_block;
}

ParserResult<Declaration> Parser::ParserClassDecl() {
  // advance class keyword
  Advance();
  ValidToken();

  std::unique_ptr<Identifier> class_name;
  std::unique_ptr<Expression> parent;

  if (token_ != TokenKind::IDENTIFIER) {
    ErrorMsg(boost::format("expected identifier got %1%")% TokenValueStr());
    return ParserResult<Declaration>(); // Error
  }

  class_name = std::move(factory_.NewIdentifier(boost::get<std::string>(
      token_.GetValue()), std::move(nullptr)));

  Advance();
  ValidToken();

  // if lparen the class has a parent class
  // if not lparen and COLON the class implements interface
  if (token_ == TokenKind::LPAREN) {
    Advance();
    ValidToken();

    ParserResult<Expression> super(ParserPostExp());
    parent = super.MoveAstNode();

    if (token_ != TokenKind::RPAREN) {
      ErrorMsg(boost::format("expected token ) got %1%")% TokenValueStr());
      return ParserResult<Declaration>(); // Error
    }

    Advance();
    ValidToken();
  }

  std::unique_ptr<ExpressionList> interfaces;

  if (token_ == TokenKind::COLON) {
    Advance();
    ValidToken();

    interfaces = ParserPostExpList().MoveAstNode();
  }

  ValidToken();
  if (token_ != TokenKind::LBRACE) {
    ErrorMsg(boost::format("expected token { got %1%")% TokenValueStr());
    return ParserResult<Declaration>(); // Error
  }

  ParserResult<ClassBlock> class_block(ParserClassBlock());

  // TODO: implement final keyword for class
  ParserResult<Declaration> class_decl(factory_.NewClassDeclaration(
      std::move(class_name), std::move(parent), std::move(interfaces),
      std::move(class_block.MoveAstNode()), false));

  return class_decl;
}

}
}
