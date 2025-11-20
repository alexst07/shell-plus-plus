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

#ifndef SHPP_ERROR_FORMATTER_H
#define SHPP_ERROR_FORMATTER_H

#include <string>
#include <sstream>
#include <iostream>
#include "run_time_error.h"
#include "msg.h"

namespace shpp {
namespace internal {

/**
 * @brief ANSI color codes for terminal output
 */
class Color {
 public:
  // Text colors
  static const std::string RED;
  static const std::string GREEN;
  static const std::string YELLOW;
  static const std::string BLUE;
  static const std::string MAGENTA;
  static const std::string CYAN;
  static const std::string WHITE;
  static const std::string BRIGHT_RED;
  static const std::string BRIGHT_BLUE;
  static const std::string BRIGHT_YELLOW;
  
  // Text styles
  static const std::string BOLD;
  static const std::string RESET;
  static const std::string DIM;
  
  // Check if colors should be enabled
  static bool should_use_color();
};

/**
 * @brief Formats error messages in a Rust/Python-like style with colors
 * 
 * Example output:
 * error: undefined variable 'x'
 *   --> test.sh:5:10
 *    |
 *  5 | print(x + 1)
 *    |       ^ undefined variable
 */
class ErrorFormatter {
 public:
  /**
   * @brief Format a complete error message with context
   * 
   * @param e The runtime error to format
   * @return Formatted error string
   */
  static std::string FormatError(const RunTimeError& e);
  
  /**
   * @brief Format a single message in the error stack
   * 
   * @param msg The message to format
   * @param is_note If true, format as a note rather than error
   * @return Formatted message string
   */
  static std::string FormatMessage(const Message& msg, bool is_note = false);
  
 private:
  /**
   * @brief Format the error header (error: message)
   */
  static std::string FormatHeader(const std::string& severity, 
                                   const std::string& message,
                                   bool use_color);
  
  /**
   * @brief Format the location line (--> file:line:col)
   */
  static std::string FormatLocation(const std::string& file, 
                                     uint line, 
                                     uint col,
                                     bool use_color);
  
  /**
   * @brief Format the source code line with line number
   */
  static std::string FormatSourceLine(const std::string& line_content, 
                                       uint line_num,
                                       bool use_color);
  
  /**
   * @brief Format the pointer/caret line pointing to the error
   */
  static std::string FormatPointer(uint col, 
                                    const std::string& hint,
                                    bool use_color);
  
  /**
   * @brief Get line number width for padding
   */
  static size_t GetLineNumWidth(uint line_num);
  
  /**
   * @brief Get the error type name from error code
   */
  static std::string GetErrorTypeName(RunTimeError::ErrorCode code);
};

}  // namespace internal
}  // namespace shpp

#endif  // SHPP_ERROR_FORMATTER_H

