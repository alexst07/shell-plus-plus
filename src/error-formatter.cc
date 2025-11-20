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

#include "error-formatter.h"
#include <cstdlib>
#include <unistd.h>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace shpp {
namespace internal {

// ANSI color codes
const std::string Color::RED = "\033[31m";
const std::string Color::GREEN = "\033[32m";
const std::string Color::YELLOW = "\033[33m";
const std::string Color::BLUE = "\033[34m";
const std::string Color::MAGENTA = "\033[35m";
const std::string Color::CYAN = "\033[36m";
const std::string Color::WHITE = "\033[37m";
const std::string Color::BRIGHT_RED = "\033[91m";
const std::string Color::BRIGHT_BLUE = "\033[94m";
const std::string Color::BRIGHT_YELLOW = "\033[93m";
const std::string Color::BOLD = "\033[1m";
const std::string Color::RESET = "\033[0m";
const std::string Color::DIM = "\033[2m";

bool Color::should_use_color() {
  // Check if stdout is a terminal
  if (!isatty(STDOUT_FILENO)) {
    return false;
  }
  
  // Check NO_COLOR environment variable
  const char* no_color = std::getenv("NO_COLOR");
  if (no_color != nullptr && no_color[0] != '\0') {
    return false;
  }
  
  // Check TERM environment variable
  const char* term = std::getenv("TERM");
  if (term == nullptr || std::string(term) == "dumb") {
    return false;
  }
  
  return true;
}

std::string ErrorFormatter::GetErrorTypeName(RunTimeError::ErrorCode code) {
  switch (code) {
    case RunTimeError::ErrorCode::NULL_ACCESS:
      return "null access";
    case RunTimeError::ErrorCode::SYMBOL_NOT_FOUND:
      return "symbol not found";
    case RunTimeError::ErrorCode::BAD_ALLOC:
      return "allocation error";
    case RunTimeError::ErrorCode::OUT_OF_RANGE:
      return "out of range";
    case RunTimeError::ErrorCode::KEY_NOT_FOUND:
      return "key not found";
    case RunTimeError::ErrorCode::ID_NOT_FOUND:
      return "id not found";
    case RunTimeError::ErrorCode::INVALID_ARGS:
      return "invalid arguments";
    case RunTimeError::ErrorCode::INCOMPATIBLE_TYPE:
      return "incompatible type";
    case RunTimeError::ErrorCode::FUNC_PARAMS:
      return "function parameters";
    case RunTimeError::ErrorCode::ZERO_DIV:
      return "division by zero";
    case RunTimeError::ErrorCode::FD_NOT_FOUND:
      return "file descriptor not found";
    case RunTimeError::ErrorCode::INVALID_OPCODE:
      return "invalid operation";
    case RunTimeError::ErrorCode::FILE:
      return "file error";
    case RunTimeError::ErrorCode::INVALID_COMMAND:
      return "invalid command";
    case RunTimeError::ErrorCode::IMPORT:
      return "import error";
    case RunTimeError::ErrorCode::ASSERT:
      return "assertion failed";
    case RunTimeError::ErrorCode::PARSER:
      return "syntax error";
    case RunTimeError::ErrorCode::REGEX:
      return "regex error";
    case RunTimeError::ErrorCode::GLOB:
      return "glob error";
    case RunTimeError::ErrorCode::EVAL:
      return "eval error";
    case RunTimeError::ErrorCode::SYMBOL_DEF:
      return "symbol definition error";
    case RunTimeError::ErrorCode::CUSTON:
      return "error";
    default:
      return "error";
  }
}

size_t ErrorFormatter::GetLineNumWidth(uint line_num) {
  if (line_num == 0) return 1;
  size_t width = 0;
  while (line_num > 0) {
    width++;
    line_num /= 10;
  }
  return width;
}

std::string ErrorFormatter::FormatHeader(const std::string& severity,
                                          const std::string& message,
                                          bool use_color) {
  std::stringstream ss;
  
  if (use_color) {
    ss << Color::BOLD << Color::BRIGHT_RED << severity << Color::RESET
       << Color::BOLD << ": " << message << Color::RESET;
  } else {
    ss << severity << ": " << message;
  }
  
  return ss.str();
}

std::string ErrorFormatter::FormatLocation(const std::string& file,
                                            uint line,
                                            uint col,
                                            bool use_color) {
  std::stringstream ss;
  
  if (use_color) {
    ss << "  " << Color::BOLD << Color::BRIGHT_BLUE << "-->" << Color::RESET
       << " " << file << ":" << line << ":" << col;
  } else {
    ss << "  --> " << file << ":" << line << ":" << col;
  }
  
  return ss.str();
}

std::string ErrorFormatter::FormatSourceLine(const std::string& line_content,
                                              uint line_num,
                                              bool use_color) {
  if (line_content.empty()) {
    return "";
  }
  
  std::stringstream ss;
  size_t width = GetLineNumWidth(line_num);
  
  // Format: " 5 | source code here"
  if (use_color) {
    ss << " " << Color::BOLD << Color::BRIGHT_BLUE
       << std::setw(width) << line_num << " |" << Color::RESET
       << " " << line_content;
  } else {
    ss << " " << std::setw(width) << line_num << " | " << line_content;
  }
  
  return ss.str();
}

std::string ErrorFormatter::FormatPointer(uint col,
                                           const std::string& hint,
                                           bool use_color) {
  std::stringstream ss;
  
  // Create the pointer line with carets
  // Format: "   | ^^^^^ hint text"
  size_t num_spaces = (col > 0) ? (col - 1) : 0;
  std::string spaces(num_spaces, ' ');
  
  if (use_color) {
    ss << "   " << Color::BOLD << Color::BRIGHT_BLUE << "|" << Color::RESET
       << " " << spaces << Color::BOLD << Color::BRIGHT_RED << "^";
    
    // Add a few more carets for emphasis
    if (hint.length() > 1) {
      size_t num_carets = std::min(hint.length() / 2, size_t(5));
      for (size_t i = 1; i < num_carets; ++i) {
        ss << "^";
      }
    }
    
    if (!hint.empty()) {
      ss << " " << hint;
    }
    
    ss << Color::RESET;
  } else {
    ss << "   | " << spaces << "^";
    if (!hint.empty()) {
      ss << " " << hint;
    }
  }
  
  return ss.str();
}

std::string ErrorFormatter::FormatError(const RunTimeError& e) {
  bool use_color = Color::should_use_color();
  std::stringstream result;
  
  // Get the error type name
  std::string error_type = GetErrorTypeName(e.err_code());
  
  // Format main error header
  result << FormatHeader(error_type, e.what(), use_color) << "\n";
  
  // Format location if we have position info
  if (e.pos().line > 0) {
    result << FormatLocation(e.file(), e.pos().line, e.pos().col, use_color) << "\n";
    
    // Add separator line
    size_t width = GetLineNumWidth(e.pos().line);
    if (use_color) {
      result << " " << std::string(width, ' ')
             << Color::BOLD << Color::BRIGHT_BLUE << " |" << Color::RESET << "\n";
    } else {
      result << " " << std::string(width, ' ') << " |\n";
    }
    
    // Add source line with pointer
    std::string source_line = FormatSourceLine(e.line_error(), e.pos().line, use_color);
    if (!source_line.empty()) {
      result << source_line << "\n";
      result << FormatPointer(e.pos().col, error_type, use_color) << "\n";
    }
  } else if (!e.file().empty()) {
    // If we have file but no position, just show file
    result << "  in " << e.file() << "\n";
  }
  
  // Format additional messages (stack trace)
  const auto& messages = const_cast<RunTimeError&>(e).messages();
  if (messages.size() > 0) {
    result << "\n";
    if (use_color) {
      result << Color::BOLD << "Traceback:" << Color::RESET << "\n";
    } else {
      result << "Traceback:\n";
    }
    
    for (const auto& msg : messages) {
      result << FormatMessage(msg, true) << "\n";
    }
  }
  
  return result.str();
}

std::string ErrorFormatter::FormatMessage(const Message& msg, bool is_note) {
  bool use_color = Color::should_use_color();
  std::stringstream result;
  
  // Format as note/info
  std::string severity = is_note ? "note" : "error";
  
  if (use_color) {
    result << "  " << Color::BOLD << Color::BRIGHT_BLUE
           << severity << Color::RESET << ": " << msg.msg() << "\n";
  } else {
    result << "  " << severity << ": " << msg.msg() << "\n";
  }
  
  // Add location
  if (msg.line() > 0) {
    result << FormatLocation(msg.file(), msg.line(), msg.pos(), use_color) << "\n";
    
    // Add source line if available
    std::string line_error = msg.line_error();
    if (!line_error.empty()) {
      size_t width = GetLineNumWidth(msg.line());
      
      if (use_color) {
        result << "   " << std::string(width, ' ')
               << Color::BOLD << Color::BRIGHT_BLUE << " |" << Color::RESET << "\n";
      } else {
        result << "   " << std::string(width, ' ') << " |\n";
      }
      
      result << " " << FormatSourceLine(line_error, msg.line(), use_color) << "\n";
      
      if (msg.pos() > 0) {
        result << " " << FormatPointer(msg.pos(), "", use_color) << "\n";
      }
    }
  }
  
  return result.str();
}

}  // namespace internal
}  // namespace shpp

