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

#ifndef SETI_EXTRACT_EXPRESSION_H
#define SETI_EXTRACT_EXPRESSION_H

#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <cstring>

#include "token.h"
#include "msg.h"
#include "parser_result.h"
#include "ast/ast.h"

namespace seti {
namespace internal {

class ExtractExpr {
 public:
  ExtractExpr(const std::string& src);

  void Extract();

  bool has_expr() {
    return start_pos_ != -1 && end_pos_ != -1;
  }

  int start_pos() {
    return start_pos_;
  }

  int end_pos() {
    return end_pos_;
  }

 private:
  inline void Advance() {
    c_ = src_[++cursor_];
  }

  bool HasNextChar() {
    return cursor_ < (src_.length() - 1);
  }

  int cursor_;
  char c_;
  const std::string& src_;
  int start_pos_;
  int end_pos_;
  int num_open_;
  bool valid_scope_;
};

ParserResult<Cmd> ParserExpr(const std::string& src);

}
}

#endif  // SETI_EXTRACT_EXPRESSION_H
