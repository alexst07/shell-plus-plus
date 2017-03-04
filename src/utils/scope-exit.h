#ifndef SHPP_SCOPE_EXIT_H
#define SHPP_SCOPE_EXIT_H

namespace shpp {
namespace internal {

template <typename T>
class ScopeExit {
 public:
  ScopeExit(T &&f) : f_{std::move(f)} {}

  ~ScopeExit() { f_(); }

 private:
  T f_;
};

template <typename T>
ScopeExit<T> MakeScopeExit(T &&f) {
  return ScopeExit<T>{std::move(f)};
}

template <typename T>
void IgnoreUnused (T const &) {}

}
}

#endif  // SHPP_SCOPE_EXIT_H

