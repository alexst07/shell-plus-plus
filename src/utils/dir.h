#ifndef SETI_DIR_H
#define SETI_DIR_H

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <string>

namespace seti {
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

#endif  // SETI_DIR_H
