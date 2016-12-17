#include "cmd-exec.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace setti {
namespace internal {

int ExecCmd(std::vector<std::string>&& args) {
  char** p_args = new char*[args.size() + 1];

  for (size_t i = 0; i < args.size(); i++) {
    p_args[i] = const_cast<char*>(args[i].data());
    std::cout << "::" << p_args[i];
  }

  p_args[args.size()] = NULL;

  int ret = execvp(p_args[0], p_args);

  delete p_args;

  return ret;
}

int WaitCmd(int pid) {
  int status;

  waitpid(pid,&status,0);
  return status;
}

}
}
