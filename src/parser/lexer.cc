#include "lexer.h"

namespace setti {
namespace internal {

void Lexer::Advance() {
  if (buffer_cursor_ == strlen_) {
    c_ = kEndOfInput;
    return;
  }

  // Check new line and ser cursor position
  if (c_ == '\n') {
    line_++;
    line_pos_ = 0;
  }

  c_ = str_[buffer_cursor_++];

  // Always increment line position, because the first char on line is '1'
  line_pos_++;
}

char Lexer::PeekAhead() {
  if ((buffer_cursor_ + 1) == strlen_)
    return kEndOfInput;

  return str_[buffer_cursor_ + 1];
}

void Lexer::Select(TokenKind k) {
  Advance();
  bool blank_after = PeekAhead() == ' ';
  Token t = Token(k, blank_after, line_, line_pos_);
  ts_.PushToken(std::move(t));
}

void Lexer::Select(TokenKind k, Token::Value v) {
  Advance();
  bool blank_after = PeekAhead() == ' ';
  Token t = Token(k, v, blank_after, line_, line_pos_);
  ts_.PushToken(std::move(t));
}

void Lexer::SkipSingleLineComment() {
  while (c_ != kEndOfInput && c_ != '\n')
    Advance();

  if (c_ == '\n')
    Advance();
}

void Lexer::ErrorMsg(const boost::format& fmt_msg) {
  Message msg(Message::Severity::ERR, fmt_msg, line_, line_pos_);
  msgs_.Push(std::move(msg));
  nerror_++;
}

char Lexer::ScanStringEscape() {
  Advance();

  char c = c_;

  // Handle special char on string
  switch (c) {
    case '\'':  // fall through
    case '"' :  // fall through
    case '\\': break;
    case 'b' : c = '\b'; break;
    case 'f' : c = '\f'; break;
    case 'n' : c = '\n'; break;
    case 'r' : c = '\r'; break;
    case 't' : c = '\t'; break;
  }

  return c;
}

void Lexer::ScanString() {
  std::string str = "";
  while(true) {
    if (c_ == '\n' || c_ == kEndOfInput) {
      ErrorMsg(boost::format("string literal not terminated"));
      break;
    }

    if (c_ == '"')
      break;

    if (c_ == '\\') {
       str += ScanStringEscape();
       continue;
    }

    str += c_;
    Advance();
  }

  Select(TokenKind::STRING_LITERAL, str);
}

void Lexer::Scanner() {
  switch (c_) {
    case '#':
      Advance();
      SkipSingleLineComment();
      break;

    case ' ':
    case '\t':
      Advance();
      break;

    case '\n':
      Advance();
      Select(TokenKind::NWL);
      break;

    case '"':
      ScanString();
      break;

    case '<':
      // < <= << <<=
      Advance();
      if (c_ == '=') {
        Select(TokenKind::LESS_EQ);
      } else if (c_ == '<') {
        Advance();
        if (c_ == '=') {
          Select(TokenKind::ASSIGN_SHL);
        } else {
          Select(TokenKind::SHL);
        }
      } else {
        Select(TokenKind::LESS_THAN);
      }
      break;

    case '>':
      // > >= >> >>=
      Advance();
      if (c_ == '=') {
        Select(TokenKind::GREATER_EQ);
      } else if (c_ == '>') {
        Advance();
        if (c_ == '=') {
          Select(TokenKind::ASSIGN_SAR);
        } else {
          Select(TokenKind::SAR);
        }

      } else {
        Select(TokenKind::GREATER_THAN);
      }
      break;

    case '=':
      // = ==
      Advance();
      if (c_ == '=') {
        Select(TokenKind::EQUAL);
      } else {
        Select(TokenKind::ASSIGN);
      }
      break;

    case '!':
      // ! !=
      Advance();
      if (c_ == '=') {
        Select(TokenKind::NOT_EQUAL);
      } else {
        Select(TokenKind::NOT);
      }
      break;

    case '+':
      // + +=
      Advance();
      if (c_ == '=') {
        Select(TokenKind::ASSIGN_ADD);
      } else {
        Select(TokenKind::ADD);
      }
      break;

    case '-':
      // - -=
      Advance();
      if (c_ == '=') {
        Select(TokenKind::ASSIGN_SUB);
      } else if (c_ == '=') {
        Select(TokenKind::ARROW);
      } else {
        Select(TokenKind::SUB);
      }
      break;

    case '*':
      // * *=
      Advance();
      if (c_ == '=') {
        Select(TokenKind::ASSIGN_MUL);
      } else {
        Select(TokenKind::MUL);
      }
      break;

    case '/':
      // * /=
      Advance();
      if (c_ == '=') {
        Select(TokenKind::ASSIGN_DIV);
      } else {
        Select(TokenKind::DIV);
      }
      break;

    case '%':
      // % %=
      Advance();
      if (c_ == '=') {
        Select(TokenKind::ASSIGN_MOD);
      } else {
        Select(TokenKind::MOD);
      }
      break;

    case '&':
      // & &= &&
      Advance();
      if (c_ == '=') {
        Select(TokenKind::ASSIGN_BIT_AND);
      } else if (c_ == '&') {
        Select(TokenKind::AND);
      } else {
        Select(TokenKind::BIT_AND);
      }
      break;

    case '|':
      // | |= ||
      Advance();
      if (c_ == '=') {
        Select(TokenKind::ASSIGN_BIT_OR);
      } else if (c_ == '&') {
        Select(TokenKind::OR);
      } else {
        Select(TokenKind::BIT_OR);
      }
      break;

    case '^':
      // ^ ^=
      Advance();
      if (c_ == '=') {
        Select(TokenKind::ASSIGN_BIT_XOR);
      } else {
        Select(TokenKind::BIT_XOR);
      }
      break;

    case ';':
      Select(TokenKind::SEMI_COLON);
      break;

    case ':':
      Select(TokenKind::COLON);
      break;

    case ',':
      Select(TokenKind::COMMA);
      break;

    case '$':
      // $ $( ${
      Advance();
      if (c_ == '(') {
        Select(TokenKind::DOLLAR_LPAREN);
      } else if (c_ == '{') {
        Select(TokenKind::DOLLAR_LBRACE);
      } else {
        Select(TokenKind::DOLLAR);
      }
      break;

    case '(':
      Select(TokenKind::LPAREN);
      break;

    case ')':
      Select(TokenKind::RPAREN);
      break;

    case '{':
      Select(TokenKind::LBRACE);
      break;

    case '}':
      Select(TokenKind::RBRACE);
      break;

    case '[':
      Select(TokenKind::LBRACKET);
      break;

    case ']':
      Select(TokenKind::RBRACKET);
      break;

    default:
      ErrorMsg(boost::format("Not recognized character: %1%")% c_);
  }
}

}
}
