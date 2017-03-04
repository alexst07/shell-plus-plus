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

#include "extract_expr.h"

#include "parser.h"
#include "run_time_error.h"

namespace shpp {
namespace internal {

ExtractExpr::ExtractExpr(const std::string& src)
    : cursor_(0)
    , c_(src[0])
    , src_(src)
    , start_pos_(-1)
    , end_pos_(-1)
    , num_open_(0)
    , valid_scope_(true) {}

void ExtractExpr::Extract() {
  while (true) {
    switch (c_) {
      case '\\':
        break;

      case '$': {
        Advance();
        if (c_ == '{' && start_pos_ == -1) {
          start_pos_ = cursor_;
          num_open_++;
        }
      } break;

      case '}': {
        if (num_open_ > 1 && valid_scope_) {
          num_open_--;
        } else if (start_pos_ >= 0 && valid_scope_) {
          end_pos_ = cursor_;
          return;
        }
      } break;

      case '{': {
        if (num_open_ > 0 && valid_scope_) {
          num_open_++;
        }
      } break;

      case '"':
        valid_scope_ = valid_scope_? false:true;
        break;
    }

    if (HasNextChar()) {
      Advance();
     } else {
      return;
    }
  }
}

ParserResult<Cmd> ParserExpr(const std::string& src) {
  Lexer l(src);
  TokenStream ts = l.Scanner();
  Parser p(std::move(ts));
  auto res = p.AstGenCmdExpr();

  if (p.nerrors() == 0) {
    return res;
  } else {
    Message msg = p.Msgs();
    throw RunTimeError(RunTimeError::ErrorCode::PARSER, msg.msg(),
                       Position{msg.line(), msg.pos()});
  }
}

}
}
