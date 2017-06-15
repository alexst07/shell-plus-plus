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

#ifndef SHPP_RUN_TIME_ERROR_H
#define SHPP_RUN_TIME_ERROR_H

#include <exception>
#include <string>
#include <boost/format.hpp>

#include "ast/ast.h"
#include "msg.h"

namespace shpp {

/**
 * @brief Class to represent an run time error
 *
 * This class encapsulates run time error, and it is
 * used to represent errors as symbol not found,
 * command not found and out of range
 */
class RunTimeError : public std::exception {
 public:
  enum class ErrorCode: uint8_t{
    NULL_ACCESS,
    SYMBOL_NOT_FOUND,
    CMD_NOT_FOUND,
    BAD_ALLOC,
    OUT_OF_RANGE,
    KEY_NOT_FOUND,
    ID_NOT_FOUND,
    INVALID_ARGS,
    INCOMPATIBLE_TYPE,
    FUNC_PARAMS,
    ZERO_DIV,
    FD_NOT_FOUND,
    INVALID_OPCODE,
    FILE,
    INVALID_COMMAND,
    IMPORT,
    ASSERT,
    PARSER,
    INTERPRETER_FILE,
    REGEX,
    GLOB,
    CUSTON
  };

  RunTimeError();

  RunTimeError(ErrorCode code, const boost::format& msg,
      const internal::Messages& msgs = internal::Messages())
      : code_(code)
      , msg_(boost::str(msg))
      , pos_{0, 0}
      , stack_msg_{msgs} {}

  RunTimeError(ErrorCode code, const boost::format& msg, internal::Position pos,
      const internal::Messages& msgs = internal::Messages())
      : code_(code)
      , msg_(boost::str(msg))
      , pos_{pos}
      , stack_msg_{msgs} {}

  RunTimeError(ErrorCode code, const std::string& msg, internal::Position pos,
      const internal::Messages& msgs = internal::Messages())
      : code_(code)
      , msg_(msg)
      , pos_{pos}
      , stack_msg_{msgs} {}

  virtual ~RunTimeError() noexcept  = default;

  RunTimeError(const RunTimeError& rt_err)
      : code_(rt_err.code_)
      , msg_(rt_err.msg_)
      , pos_(rt_err.pos_)
      , stack_msg_(rt_err.stack_msg_)
      , str_line_error_(rt_err.str_line_error_)
      , file_(rt_err.file_) {}

  RunTimeError& operator=(const RunTimeError& rt_err) {
    code_ = rt_err.code_;
    msg_ = rt_err.msg_;
    pos_ = rt_err.pos_;
    stack_msg_ = rt_err.stack_msg_;
    str_line_error_ = rt_err.str_line_error_;
    file_ = rt_err.file_;

    return *this;
  }

  /**
   * @return the error description and the context as a text string.
   */
  virtual const char* what() const noexcept {
    return msg_.c_str();
  }

  ErrorCode err_code() const noexcept {
    return code_;
  }

  const std::string& msg() const noexcept {
    return msg_;
  }

  internal::Position pos() const noexcept {
    return pos_;
  }

  RunTimeError& AppendMsg(internal::Message&& msg) {
    stack_msg_.Push(std::move(msg));
    return *this;
  }

  internal::Messages& messages() {
    return stack_msg_;
  }

  void file(const std::string& str_file) {
    file_ = str_file;
  }

  std::string file() const {
    return file_;
  }

  void line_error(const std::string& str_line) {
    str_line_error_ = str_line;
  }

  std::string line_error() const {
    return str_line_error_;
  }

  ErrorCode code_;
  std::string msg_;
  internal::Position pos_;
  internal::Messages stack_msg_;
  std::string str_line_error_;
  std::string file_;
};

}

#endif  // SHPP_RUN_TIME_ERROR_H
