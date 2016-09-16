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
  bool blank_after = PeekAhead() == ' ';
  Token t = Token(k, blank_after, line_, line_pos_);
  ts_.PushToken(std::move(t));
}

void Lexer::Select(TokenKind k, Token::Value v) {
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

void Lexer::ErrorMsg(const std::string str_msg) {
  Message msg(Message::Severity::ERR, str_msg, line_, line_pos_);
  msgs_.Push(std::move(msg));
}

void Lexer::ScanStringEscape() {
  char c = c_;

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
}

void Lexer::ScanString() {
  while(true) {
    if (c_ == '\n' || c_ == kEndOfInput) {
      ErrorMsg("string literal not terminated");
      break;
    }

    if (c_ == '"')
      break;

    if (c_ == '\\')
      ScanStringEscape();
  }

  Advance();
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

//     case '"':

  }
}

}
}
