#ifndef SETI_RUN_TIME_ERROR_H
#define SETI_RUN_TIME_ERROR_H

#include <exception>
#include <string>
#include <boost/format.hpp>

#include "ast/ast.h"

namespace setti {
namespace internal {

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

  RunTimeError(ErrorCode code, const boost::format& msg, Position pos)
      : code_(code), msg_(boost::str(msg)), pos_{pos} {}

  RunTimeError(ErrorCode code, const std::string& msg, Position pos)
      : code_(code), msg_(msg), pos_{pos} {}

  virtual ~RunTimeError() noexcept  = default;

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

  Position pos() const noexcept {
    return pos_;
  }

  ErrorCode code_;
  std::string msg_;
  Position pos_;
};

}
}

#endif  // SETI_RUN_TIME_ERROR_H
