#include "parser.h"

#include <sstream>

namespace setti {
namespace internal {

ParserResult<Ast> Parser::ParserPostExpr() {
  ParserResult<Ast> res = ParserPrimaryExpr();

  if (CurrentToken().Is(TokenKind::ARROW)) {
    return ParserPostExpr();
  } else (CurrentToken().Is(TokenKind::LBRACKET))
}

ParserResult<Ast> Parser::ParserPrimaryExpr() {
  const Token token(NextToken());
  if (token.Is(TokenKind::INT_LITERAL)) {
    return ParserResult<Ast>();
  } else if (token.Is(TokenKind::STRING_LITERAL)) {
    return ParserResult<Ast>();
  } else if (token.Is(TokenKind::REAL_LITERAL)) {
    return ParserResult<Ast>();
  } else if (token.Is(TokenKind::KW_TRUE)) {
    return ParserResult<Ast>();
  } else if (token.Is(TokenKind::KW_FALSE)) {
    return ParserResult<Ast>();
  } else if (token.Is(TokenKind::IDENTIFIER)) {
    return ParserResult<Ast>();
  } else {
    return ParserResult<Ast>(); // Error
  }
}

}
}
