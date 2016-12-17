#include "cmd-exec.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace setti {
namespace internal {

int ExecCmd(std::vector<std::string>&& args) {
  std::unique_ptr<char*[]> p_args(new char*[args.size()]);

  for (size_t i = 0; i < args.size(); i++) {
    p_args[i] = const_cast<char*>(args[i].data());
  }

  return execvp(p_args.get()[0], p_args.get());
}

int WaitCmd(int pid) {
  int status;

  waitpid(pid,&status,0);
  return status;
}

}
}
