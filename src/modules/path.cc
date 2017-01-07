#include "path.h"

#include <boost/filesystem.hpp>
#include <unistd.h>

namespace setti {
namespace internal {
namespace module {
namespace path {

ObjectPtr PwdFunc::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NO_PARAMS(params)

  namespace fs = boost::filesystem;

  fs::path path = fs::current_path();
  return obj_factory_.NewString(path.string());
}

ObjectPtr ExistsFunc::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1)

  if (params[0]->type() != ObjectType::STRING) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,                   \
                       boost::format("path is not string"));
  }

  namespace fs = boost::filesystem;

  std::string str_path = static_cast<StringObject&>(*params[0]).value();
  fs::path path{str_path};

  bool exists = fs::exists(path);
  return obj_factory_.NewBool(exists);
}

ObjectPtr IsRegularFile::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1)

  if (params[0]->type() != ObjectType::STRING) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,                   \
                       boost::format("path is not string"));
  }

  namespace fs = boost::filesystem;

  std::string str_path = static_cast<StringObject&>(*params[0]).value();
  fs::path path{str_path};

  bool v = fs::is_regular_file(path);
  return obj_factory_.NewBool(v);
}

ObjectPtr IsDirFunc::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1)

  if (params[0]->type() != ObjectType::STRING) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,                   \
                       boost::format("path is not string"));
  }

  namespace fs = boost::filesystem;

  std::string str_path = static_cast<StringObject&>(*params[0]).value();
  fs::path path{str_path};

  bool v = fs::is_directory(path);
  return obj_factory_.NewBool(v);
}

ObjectPtr IsSymLink::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1)

  if (params[0]->type() != ObjectType::STRING) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,                   \
                       boost::format("path is not string"));
  }

  namespace fs = boost::filesystem;

  std::string str_path = static_cast<StringObject&>(*params[0]).value();
  fs::path path{str_path};

  bool v = fs::is_symlink(path);
  return obj_factory_.NewBool(v);
}

ObjectPtr IsReadable::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1)

  if (params[0]->type() != ObjectType::STRING) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,                   \
                       boost::format("path is not string"));
  }

  namespace fs = boost::filesystem;

  std::string str_path = static_cast<StringObject&>(*params[0]).value();
  if (access(str_path.c_str(), R_OK) < 0) {
    return obj_factory_.NewBool(false);
  } else {
    return obj_factory_.NewBool(true);
  }
}

ObjectPtr IsWritable::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1)

  if (params[0]->type() != ObjectType::STRING) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,                   \
                       boost::format("path is not string"));
  }

  namespace fs = boost::filesystem;

  std::string str_path = static_cast<StringObject&>(*params[0]).value();
  if (access(str_path.c_str(), W_OK) < 0) {
    return obj_factory_.NewBool(false);
  } else {
    return obj_factory_.NewBool(true);
  }
}

ObjectPtr IsExecutable::Call(Executor*, std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1)

  if (params[0]->type() != ObjectType::STRING) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,                   \
                       boost::format("path is not string"));
  }

  namespace fs = boost::filesystem;

  std::string str_path = static_cast<StringObject&>(*params[0]).value();
  if (access(str_path.c_str(), X_OK) < 0) {
    return obj_factory_.NewBool(false);
  } else {
    return obj_factory_.NewBool(true);
  }
}

}
}
}
}
