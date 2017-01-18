#include "path.h"

#include <boost/filesystem.hpp>
#include <unistd.h>
#include <sys/stat.h>

#include "utils/check.h"

namespace seti {
namespace internal {
namespace module {
namespace path {

ObjectPtr PwdFunc::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NO_PARAMS(params, pwd)

  namespace fs = boost::filesystem;

  fs::path path = fs::current_path();
  return obj_factory_.NewString(path.string());
}

ObjectPtr ExistsFunc::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1, exists)
  SETI_FUNC_CHECK_PARAM_TYPE(params[0], path, STRING)

  namespace fs = boost::filesystem;

  std::string str_path = static_cast<StringObject&>(*params[0]).value();
  fs::path path{str_path};

  bool exists = fs::exists(path);
  return obj_factory_.NewBool(exists);
}

ObjectPtr IsRegularFile::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1, is_regular_file)
  SETI_FUNC_CHECK_PARAM_TYPE(params[0], path, STRING)

  namespace fs = boost::filesystem;

  std::string str_path = static_cast<StringObject&>(*params[0]).value();
  fs::path path{str_path};

  bool v = fs::is_regular_file(path);
  return obj_factory_.NewBool(v);
}

ObjectPtr IsDirFunc::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1, is_dir)
  SETI_FUNC_CHECK_PARAM_TYPE(params[0], path, STRING)

  namespace fs = boost::filesystem;

  std::string str_path = static_cast<StringObject&>(*params[0]).value();
  fs::path path{str_path};

  bool v = fs::is_directory(path);
  return obj_factory_.NewBool(v);
}

ObjectPtr IsSymLink::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1, is_sym_link)
  SETI_FUNC_CHECK_PARAM_TYPE(params[0], path, STRING)

  namespace fs = boost::filesystem;

  std::string str_path = static_cast<StringObject&>(*params[0]).value();
  fs::path path{str_path};

  bool v = fs::is_symlink(path);
  return obj_factory_.NewBool(v);
}

ObjectPtr IsReadable::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1, is_readable)
  SETI_FUNC_CHECK_PARAM_TYPE(params[0], path, STRING)

  namespace fs = boost::filesystem;

  std::string str_path = static_cast<StringObject&>(*params[0]).value();
  if (access(str_path.c_str(), R_OK) < 0) {
    return obj_factory_.NewBool(false);
  } else {
    return obj_factory_.NewBool(true);
  }
}

ObjectPtr IsWritable::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1, is_writable)
  SETI_FUNC_CHECK_PARAM_TYPE(params[0], path, STRING)

  namespace fs = boost::filesystem;

  std::string str_path = static_cast<StringObject&>(*params[0]).value();
  if (access(str_path.c_str(), W_OK) < 0) {
    return obj_factory_.NewBool(false);
  } else {
    return obj_factory_.NewBool(true);
  }
}

ObjectPtr IsExecutable::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1, is_executable)
  SETI_FUNC_CHECK_PARAM_TYPE(params[0], path, STRING)

  namespace fs = boost::filesystem;

  std::string str_path = static_cast<StringObject&>(*params[0]).value();
  if (access(str_path.c_str(), X_OK) < 0) {
    return obj_factory_.NewBool(false);
  } else {
    return obj_factory_.NewBool(true);
  }
}

ObjectPtr OwnerUid::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1, is_executable)
  SETI_FUNC_CHECK_PARAM_TYPE(params[0], path, STRING)

  namespace fs = boost::filesystem;

  struct stat sb;

  std::string str_path = static_cast<StringObject&>(*params[0]).value();

  if (stat(str_path.c_str(), &sb) == -1) {
    throw RunTimeError(RunTimeError::ErrorCode::FILE,
                       boost::format("%1%")%strerror(errno));
  }

  return obj_factory_.NewInt(sb.st_uid);
}

ObjectPtr OwnerGid::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1, is_executable)
  SETI_FUNC_CHECK_PARAM_TYPE(params[0], path, STRING)

  namespace fs = boost::filesystem;

  struct stat sb;

  std::string str_path = static_cast<StringObject&>(*params[0]).value();

  if (stat(str_path.c_str(), &sb) == -1) {
    throw RunTimeError(RunTimeError::ErrorCode::FILE,
                       boost::format("%1%")%strerror(errno));
  }

  return obj_factory_.NewInt(sb.st_gid);
}

}
}
}
}
