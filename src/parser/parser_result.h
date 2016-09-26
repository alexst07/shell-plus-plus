#ifndef SETTI_PARSER_RESULT_H
#define SETTI_PARSER_RESULT_H

#include <string>
#include <memory>
#include <vector>
#include <iostream>

#include "token.h"
#include "msg.h"
#include "lexer.h"

namespace setti {
namespace internal {

template<class T>
class ParserResult {
 public:
  explicit ParserResult(std::unique_ptr<T>&& uptr) noexcept: uptr_(std::move(uptr)) {}

  constexpr ParserResult() noexcept: uptr_(nullptr) {}
  constexpr ParserResult(nullptr_t) noexcept : ParserResult() {}
  explicit ParserResult(T* p) noexcept: uptr_(p) {}
  ParserResult(ParserResult&& pres) noexcept: uptr_(std::move(pres.uptr_)) {}
  ParserResult(const ParserResult&) = delete;

  ParserResult& operator=(ParserResult&& pres) noexcept {
    uptr_ = std::move(pres.uptr_);
  }

  ParserResult& operator=(std::unique_ptr<T>&& uptr) noexcept {
    uptr_ = std::move(uptr);
  }

  ParserResult& operator=(nullptr_t) noexcept {
    uptr_ = nullptr;
  }

  ParserResult& operator= (const ParserResult&) = delete;

  std::unique_ptr<T> MoveAstNode() noexcept {
    return std::move(uptr_);
  }

  T* NodePtr() noexcept {
    return uptr_.get();
  }

  operator bool() const {
    if (uptr_.get() != nullptr) {
      return true;
    }

    return false;
  }


 private:
  std::unique_ptr<T> uptr_;
};

}
}

#endif  // SETTI_PARSER_RESULT_H


