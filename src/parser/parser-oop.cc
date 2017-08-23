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

ParserResult<Declaration> Parser::ParserInterfaceDecl() {
  // advance interface keyword
  Advance();
  ValidToken();

  std::unique_ptr<Identifier> iface_name;

  if (token_ != TokenKind::IDENTIFIER) {
    ErrorMsg(boost::format("expected identifier got %1%")% TokenValueStr());
    return ParserResult<Declaration>(); // Error
  }

  iface_name = std::move(factory_.NewIdentifier(boost::get<std::string>(
      token_.GetValue()), std::move(nullptr)));

  Advance();
  ValidToken();

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

  ParserResult<InterfaceBlock> iface_block(ParserInterfaceBlock());

  ParserResult<Declaration> iface_decl(factory_.NewInterfaceDeclaration(
      std::move(iface_name), std::move(interfaces),
      std::move(iface_block.MoveAstNode())));

  return iface_decl;
}

ParserResult<InterfaceBlock> Parser::ParserInterfaceBlock() {
  // advance lbrace
  Advance();

  std::vector<std::unique_ptr<AstNode>> decl_list;

  while (ValidToken().IsNot(TokenKind::EOS, TokenKind::RBRACE)) {
    ValidToken();

    switch (token_.GetKind()) {
      case TokenKind::KW_FUNC: {
        ParserResult<AstNode> func(ParserFunctionDeclaration(false, true));
        decl_list.push_back(func.MoveAstNode());
      } break;

      default:
        ErrorMsg(boost::format("declaration expected, got %1%")
            %TokenValueStr());
        return ParserResult<InterfaceBlock>(); // Error
    }
  }

  if (ValidToken() != TokenKind::RBRACE) {
    ErrorMsg(boost::format("expected } token, got %1%")% TokenValueStr());
      return ParserResult<InterfaceBlock>(); // Error
  }

  Advance();
  ValidToken();

  std::unique_ptr<InterfaceDeclList> iface_list(factory_.NewInterfaceDeclList(
      std::move(decl_list)));

  ParserResult<InterfaceBlock> iface_block(factory_.NewInterfaceBlock(
      std::move(iface_list)));

  return iface_block;
}

ParserResult<ClassBlock> Parser::ParserClassBlock() {
  // advance lbrace
  Advance();

  std::vector<std::unique_ptr<AstNode>> decl_list;

  while (ValidToken().IsNot(TokenKind::EOS, TokenKind::RBRACE)) {
    ValidToken();

    switch (token_.GetKind()) {
      case TokenKind::KW_FUNC: {
        ParserResult<AstNode> func(ParserFunctionDeclaration(false, false));
        decl_list.push_back(func.MoveAstNode());
      } break;

      case TokenKind::KW_CLASS: {
        ParserResult<Declaration> class_decl(ParserClassDecl(false, false));
        decl_list.push_back(class_decl.MoveAstNode());
      } break;

      case TokenKind::KW_FINAL: {
        Advance();
        ParserResult<Declaration> class_decl(ParserClassDecl(true, false));
        decl_list.push_back(class_decl.MoveAstNode());
      } break;

      case TokenKind::KW_ABSTRACT: {
        Advance();

        if (token_ == TokenKind::KW_FUNC) {
          ParserResult<AstNode> func(ParserFunctionDeclaration(false, true));
          decl_list.push_back(func.MoveAstNode());
        } else if (token_ == TokenKind::KW_CLASS) {
          ParserResult<Declaration> class_decl(ParserClassDecl(false, true));
          decl_list.push_back(class_decl.MoveAstNode());
        } else {
          ErrorMsg(boost::format("not a valid token after abstract '%1%'")
            %TokenValueStr());
        }
      } break;

      case TokenKind::KW_STATIC: {
        // advance static keyword
        Advance();

        ParserResult<AstNode> func(ParserFunctionDeclaration(false, false, true));
        decl_list.push_back(func.MoveAstNode());
      } break;

      default:
        ErrorMsg(boost::format("declaration expected, got %1%")
            %TokenValueStr());
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

ParserResult<Declaration> Parser::ParserClassDecl(bool is_final,
    bool abstract) {
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
      std::move(class_block.MoveAstNode()), is_final, abstract));

  return class_decl;
}

}
}
