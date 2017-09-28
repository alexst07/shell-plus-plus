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

  if (token_ != TokenKind::IDENTIFIER) {
    ErrorMsg(boost::format("expected identifier got %1%")% TokenValueStr());
    return ParserResult<Declaration>(); // Error
  }

  std::unique_ptr<Identifier> iface_name = factory_.NewIdentifier(
      boost::get<std::string>(token_.GetValue()), std::move(nullptr));

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
      iface_block.MoveAstNode()));

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

      case TokenKind::KW_VAR: {
        ParserResult<Declaration> var(ParserVariableDecl());
        decl_list.push_back(var.MoveAstNode());
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

  std::unique_ptr<Expression> parent;

  if (token_ != TokenKind::IDENTIFIER) {
    ErrorMsg(boost::format("expected identifier got %1%")% TokenValueStr());
    return ParserResult<Declaration>(); // Error
  }

  std::unique_ptr<Identifier> class_name = factory_.NewIdentifier(
      boost::get<std::string>(token_.GetValue()), std::move(nullptr));

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

  ParserResult<Declaration> class_decl(factory_.NewClassDeclaration(
      std::move(class_name), std::move(parent), std::move(interfaces),
      class_block.MoveAstNode(), is_final, abstract));

  return class_decl;
}

ParserResult<Declaration> Parser::ParserVariableDecl() {
  // advance var keyword
  Advance();

  if (token_ != TokenKind::IDENTIFIER) {
    ErrorMsg(boost::format("expected identifier got %1%")% TokenValueStr());
    return ParserResult<Declaration>(); // Error
  }

  std::unique_ptr<Identifier> var_name = factory_.NewIdentifier(
      boost::get<std::string>(token_.GetValue()), std::move(nullptr));

  Advance();

  if (token_ != TokenKind::ASSIGN) {
    ErrorMsg(boost::format("expected assignment operator, got %1%")
        %TokenValueStr());
    return ParserResult<Declaration>(); // Error
  }

  Advance();

  ParserResult<AssignableValue> value = ParserAssignable();

  ParserResult<Declaration> var_decl(factory_.NewVariableDeclaration(
      std::move(var_name), value.MoveAstNode()));

  return var_decl;
}

std::unique_ptr<FinallyStatement> Parser::ParserFinally() {
  // Advance finally keyword
  Advance();
  ValidToken();

  ParserResult<Statement> finally_block(ParserBlock());
  return factory_.NewFinallyStatement(finally_block.MoveAstNode<Block>());
}

std::vector<std::unique_ptr<CatchStatement>> Parser::ParserCatchList() {
  std::vector<std::unique_ptr<CatchStatement>> catch_list;
  std::unique_ptr<Identifier> var_name;

  while (ValidToken() == TokenKind::KW_CATCH) {
    // Advance catch
    Advance();

    ParserResult<ExpressionList> exp_list = ParserExpList();

    if (token_ == TokenKind::KW_AS) {
      Advance();

      if (token_ != TokenKind::IDENTIFIER) {
        ErrorMsg(boost::format("expected identifier got %1%")%TokenValueStr());
        throw std::invalid_argument("expected identifier");
      }

      var_name = factory_.NewIdentifier(boost::get<std::string>(
          token_.GetValue()), std::move(nullptr));

      Advance();
    }

    ParserResult<Statement> catch_block(ParserBlock());

    std::unique_ptr<CatchStatement> catch_stmt = factory_.NewCatchStatement(
        exp_list.MoveAstNode(), catch_block.MoveAstNode<Block>(),
        std::move(var_name));

    catch_list.push_back(std::move(catch_stmt));
  }

  return catch_list;
}

ParserResult<Statement> Parser::ParserTryCatch() {
  // advance try keyword
  Advance();
  ValidToken();

  ParserResult<Statement> try_block(ParserBlock());

  std::vector<std::unique_ptr<CatchStatement>> catch_list;

  // check the next token, it must be catch or finally, if the next
  // token in finally, so it is the end of try catch finally block
  ValidToken();
  if (token_ == TokenKind::KW_FINALLY) {
    std::unique_ptr<FinallyStatement> finally = ParserFinally();
    return ParserResult<Statement>(factory_.NewTryCatchStatement(
        try_block.MoveAstNode<Block>(), std::move(catch_list),
        std::move(finally)));
  }

  if (token_ != TokenKind::KW_CATCH) {
    ErrorMsg(boost::format("expected catch keyword, got '%1%'")
        %TokenValueStr());
    return ParserResult<Statement>(); // Error
  }

  try {
    catch_list = ParserCatchList();
  } catch (std::invalid_argument& e) {
    return ParserResult<Statement>();
  }

  if (token_ == TokenKind::KW_FINALLY) {
    std::unique_ptr<FinallyStatement> finally = ParserFinally();
    return ParserResult<Statement>(factory_.NewTryCatchStatement(
        try_block.MoveAstNode<Block>(), std::move(catch_list),
        std::move(finally)));
  }

  std::unique_ptr<FinallyStatement> finally;
  return ParserResult<Statement>(factory_.NewTryCatchStatement(
        try_block.MoveAstNode<Block>(), std::move(catch_list),
        std::move(finally)));
}

ParserResult<Statement> Parser::ParserThrow() {
  // advance throw keyword
  Advance();

  ParserResult<Expression> exp = ParserLetExp();

  return ParserResult<Statement>(factory_.NewThrowStatement(
      exp.MoveAstNode()));
}

}
}
