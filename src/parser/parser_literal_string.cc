#include "parser_literal_string.h"

namespace shpp {
namespace internal {

ParserLiteralStringError::ParserLiteralStringError() {}

void ParserLiteralString::Scanner() {
  while (!isEnd()) {
    if (interpreter_mode_) {
      switch (getChar()) {
        case '\'':
        case '"':
          if (cmd_mode_) {
            nextAndPushChar();
          } else {
            string_inside_mode_ = !string_inside_mode_;
            nextAndPushChar();
          }
          break;

        case '\\':
          if (string_inside_mode_ || cmd_mode_) {
            nextAndPushChar();
            nextAndPushChar();
          } else {
            nextAndPushChar();
          }
          break;

        case '{':
          if (string_inside_mode_ || cmd_mode_) {
            nextAndPushChar();
          } else {
            nextAndPushChar();
            ++braces_count_;
          }
          break;

        case '}':
          if (string_inside_mode_ || cmd_mode_) {
            nextAndPushChar();
          } else {
            --braces_count_;
            if (braces_count_ == 0) {
              interpreter_mode_ = false;
              pushStrToken(true);
              pos_++;
            } else {
              nextAndPushChar();
            }
          }
          break;

        case '$':
          nextAndPushChar();
          if (getChar() == '(') {
            cmd_mode_ = true;
            nextAndPushChar();
          }
          break;

        case ')':
          if (cmd_mode_) {
            cmd_mode_ = !cmd_mode_;
          }
          nextAndPushChar();
          break;

        default:
          nextAndPushChar();
          break;
      }

      if (isEnd() && interpreter_mode_) {
        throw ParserLiteralStringError();
      }
    } else {
      parserNotInterpretableStr();
    }
  }
}

void ParserLiteralString::parserNotInterpretableStr() {
  switch (getChar()) {
    case '\\':
      nextAndPushChar();
      nextAndPushChar();
      break;

    case '{':
      interpreter_mode_ = true;
      ++braces_count_;
      pushStrToken(false);
      pos_++;
      break;

    default:
      nextAndPushChar();
      break;
  }

  if (isEnd()) {
    if (interpreter_mode_) {
      throw ParserLiteralStringError();
    }
    pushStrToken(false);
  }
}

}  // namespace internal
}  // namespace shpp