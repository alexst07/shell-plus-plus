#ifndef SHPP_DIR_H
#define SHPP_DIR_H

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <string>

namespace shpp {
namespace internal {

std::string GetHome() {
  const char *homedir;

  if ((homedir = getenv("HOME")) == NULL) {
      homedir = getpwuid(getuid())->pw_dir;
  }

  return std::string(homedir);
}

}
}

#endif  // SHPP_DIR_H
