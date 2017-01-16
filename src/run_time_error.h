#ifndef SETI_RUN_TIME_ERROR_H
#define SETI_RUN_TIME_ERROR_H

#include <exception>
#include <string>
#include <boost/format.hpp>

#include "ast/ast.h"
#include "msg.h"

namespace setti {

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
    OUT_OF_RANGE,
    KEY_NOT_FOUND,
    INCOMPATIBLE_TYPE,
    FUNC_PARAMS,
    ZERO_DIV,
    INVALID_OPCODE,
    FILE,
    INVALID_COMMAND,
    IMPORT
  };

  RunTimeError();

  RunTimeError(ErrorCode code, const boost::format& msg)
      : code_(code), msg_(boost::str(msg)), pos_{0, 0} {}

  RunTimeError(ErrorCode code, const boost::format& msg, internal::Position pos)
      : code_(code), msg_(boost::str(msg)), pos_{pos} {}

  RunTimeError(ErrorCode code, const std::string& msg, internal::Position pos)
      : code_(code), msg_(msg), pos_{pos} {}

  virtual ~RunTimeError() noexcept  = default;

  RunTimeError(const RunTimeError& rt_err)
      : code_(rt_err.code_)
      , msg_(rt_err.msg_)
      , pos_(rt_err.pos_)
      , stack_msg_(rt_err.stack_msg_) {}

  RunTimeError& operator=(const RunTimeError& rt_err) {
    code_ = rt_err.code_;
    msg_ = rt_err.msg_;
    pos_ = rt_err.pos_;
    stack_msg_ = rt_err.stack_msg_;

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

  void AppendMsg(internal::Message&& msg) {
    stack_msg_.Push(std::move(msg));
  }

  internal::Messages& messages() {
    return stack_msg_;
  }

  ErrorCode code_;
  std::string msg_;
  internal::Position pos_;
  internal::Messages stack_msg_;
};

}

#endif  // SETI_RUN_TIME_ERROR_H
