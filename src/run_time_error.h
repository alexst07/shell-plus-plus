#ifndef SETI_RUN_TIME_ERROR_H
#define SETI_RUN_TIME_ERROR_H

#include <exception>
#include <string>
#include <boost/format.hpp>

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
    ZERO_DIV
  };

  RunTimeError();

  RunTimeError(ErrorCode code, const boost::format& msg)
      : code_(code), msg_(boost::str(msg)) {}

  virtual ~RunTimeError() noexcept  = default;

  /**
   * @return the error description and the context as a text string.
   */
  virtual const char* what() const noexcept {
    msg_.c_str();
  }

  ErrorCode code_;
  std::string msg_;
};

}
}

#endif  // SETI_RUN_TIME_ERROR_H
