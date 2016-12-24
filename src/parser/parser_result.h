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
  explicit ParserResult(std::unique_ptr<T>&& uptr) noexcept
      : uptr_(std::move(uptr)) {}

  template<class U>
  explicit ParserResult(std::unique_ptr<U>&& uptr) noexcept
      : uptr_(std::move(std::unique_ptr<T>(
            static_cast<T*>(uptr.release())))) {}

  ParserResult() noexcept: uptr_(nullptr) {}
  ParserResult(std::nullptr_t) noexcept : uptr_(nullptr) {}
  explicit ParserResult(T* p) noexcept: uptr_(p) {}
  ParserResult(ParserResult&& pres) noexcept: uptr_(std::move(pres.uptr_)) {}
  ParserResult(const ParserResult&) = delete;

  ~ParserResult() {}

  ParserResult& operator=(ParserResult&& pres) noexcept {
    uptr_ = std::move(pres.uptr_);
    return *this;
  }

  ParserResult& operator=(std::unique_ptr<T>&& uptr) noexcept {
    uptr_ = std::move(uptr);
    return *this;
  }

  ParserResult& operator=(std::nullptr_t) noexcept {
    uptr_ = nullptr;
    return *this;
  }

  ParserResult& operator= (const ParserResult&) = delete;

  template <class U>
  std::unique_ptr<U> MoveAstNode() noexcept {
    auto d = static_cast<U*>(uptr_.release());
    return std::unique_ptr<U>(d);
  }

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


