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

#include "file-object.h"

#include "obj-type.h"
#include "object-factory.h"
#include "utils/check.h"

namespace shpp {
namespace internal {

FileIterObject::FileIterObject(ObjectPtr file_obj, ObjectPtr obj_type,
    SymbolTableStack&& sym_table)
    : BaseIter(ObjectType::FILE_ITER, obj_type, std::move(sym_table))
    , file_obj_(file_obj) {}

ObjectPtr FileIterObject::Next() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewString(line_);
}

ObjectPtr FileIterObject::HasNext() {
  FileObject& file = static_cast<FileObject&>(*file_obj_);
  auto& has = std::getline(file.fs(), line_);

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(bool(has));
}

ObjectPtr FileIterType::Constructor(Executor* /*parent*/,
                                     std::vector<ObjectPtr>&& params) {
  if (params.size() != 1) {
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                       boost::format("file_iter() takes exactly 1 argument"));
  }

  if (params[0]->type() != ObjectType::FILE) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("invalid type for file_iter"));
  }

  ObjectFactory obj_factory(symbol_table_stack());
  ObjectPtr obj(obj_factory.NewFileIter(params[0]));
  return obj;
}

FileObject::FileObject(const std::string& path, std::ios_base::openmode mode,
    ObjectPtr obj_type, SymbolTableStack&& sym_table)
    : Object(ObjectType::FILE, obj_type, std::move(sym_table))
    , fs_(path, mode) {}

int FileObject::Size() try {
  int pos = fs_.tellg();
  fs_.seekg(0, fs_.end);
  int length = fs_.tellg();
  fs_.seekg(pos, fs_.beg);

  return length;
} catch (std::ios_base::failure& e) {
  throw RunTimeError(RunTimeError::ErrorCode::FILE,
                     boost::format("file descriptor closed"));
}

ObjectPtr FileObject::ObjBool() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(bool(fs_));
}

ObjectPtr FileObject::Not() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(!bool(fs_));
}

ObjectPtr FileObject::ObjIter(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewFileIter(obj);
}

void FileObject::Close() try {
  fs_.close();
} catch (std::ios_base::failure& e) {
  throw RunTimeError(RunTimeError::ErrorCode::FILE,
                     boost::format("file descriptor closed"));
}

std::string FileObject::Read(int len) try {
  std::string buffer;
  buffer.resize(len+1);
  fs_.read(const_cast<char*>(buffer.data()), len);
  return buffer;
} catch (std::ios_base::failure& e) {
  throw RunTimeError(RunTimeError::ErrorCode::FILE,
                     boost::format("file descriptor closed"));
}

std::string FileObject::ReadAll() try {
  std::stringstream buffer;
  buffer << fs_.rdbuf();

  return buffer.str();
} catch (std::ios_base::failure& e) {
  throw RunTimeError(RunTimeError::ErrorCode::FILE,
                     boost::format("file descriptor problem"));
}

void FileObject::Write(const std::string& str) try {
  fs_.write(str.c_str(), str.length());
} catch (std::ios_base::failure& e) {
  throw RunTimeError(RunTimeError::ErrorCode::FILE,
                     boost::format("file descriptor closed"));
}

int FileObject::Tellg() try {
  int pos = fs_.tellg();
  return pos;
} catch (std::ios_base::failure& e) {
  throw RunTimeError(RunTimeError::ErrorCode::FILE,
                     boost::format("file descriptor closed"));
}

void FileObject::Seekg(std::streamoff off, std::ios_base::seekdir way) try {
  fs_.seekg(off, way);
} catch (std::ios_base::failure& e) {
  throw RunTimeError(RunTimeError::ErrorCode::FILE,
                     boost::format("file descriptor closed"));
}

ObjectPtr FileObject::Attr(std::shared_ptr<Object> self,
                           const std::string& name) {
  ObjectPtr obj_type = ObjType();
  return static_cast<TypeObject&>(*obj_type).CallObject(name, self);
}

FileType::FileType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
    : TypeObject("file", obj_type, std::move(sym_table)) {
  RegisterMethod<FileCloseFunc>("close", symbol_table_stack(), *this);
  RegisterMethod<FileReadLineFunc>("readline", symbol_table_stack(), *this);
  RegisterMethod<FileReadFunc>("read", symbol_table_stack(), *this);
  RegisterMethod<FileReadAllFunc>("readall", symbol_table_stack(), *this);
  RegisterMethod<FileSizeFunc>("size", symbol_table_stack(), *this);
  RegisterMethod<FileWriteFunc>("write", symbol_table_stack(), *this);
  RegisterMethod<FileTellgFunc>("tellg", symbol_table_stack(), *this);
  RegisterMethod<FileSeekgFunc>("seekg", symbol_table_stack(), *this);
}

ObjectPtr FileType::Constructor(Executor* /*parent*/,
                               std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS_AT_LEAST(params, 1, file)
  SHPP_FUNC_CHECK_NUM_PARAMS_UNTIL(params, 2, file)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[0], path, STRING)

  std::ios_base::openmode mode = std::fstream::in | std::fstream::out;

  const std::string& path = static_cast<StringObject&>(*params[0]).value();

  // if the second argument was set, choose a mode open file based on python
  // standard
  if (params.size() == 2) {
    SHPP_FUNC_CHECK_PARAM_TYPE(params[1], mode, STRING)
    std::string str_mode = static_cast<StringObject&>(*params[1]).value();

    if (str_mode == "r") {
      mode = std::fstream::in;
    } else if (str_mode == "rb") {
      mode = std::fstream::in | std::fstream::binary;
    } else if (str_mode == "r+") {
      mode = std::fstream::in | std::fstream::out;
    } else if (str_mode == "rb+") {
      mode = std::fstream::in | std::fstream::out | std::fstream::binary;
    } else if (str_mode == "w") {
      mode = std::fstream::out;
    } else if (str_mode == "wb") {
      mode = std::fstream::out | std::fstream::binary;
    } else if (str_mode == "w+") {
      mode = std::fstream::in | std::fstream::out | std::fstream::trunc;
    } else if (str_mode == "a") {
      mode = std::fstream::out | std::fstream::app;
    } else if (str_mode == "ab") {
      mode =std::fstream::out | std::fstream::app | std::fstream::binary;
    } else if (str_mode == "a+") {
      mode = std::fstream::out | std::fstream::app | std::fstream::in;
    } else if (str_mode == "ab+") {
      mode = std::fstream::out | std::fstream::app | std::fstream::in
           | std::fstream::binary;
    } else {
      throw RunTimeError(RunTimeError::ErrorCode::INVALID_ARGS,
                         boost::format("invalid file mode: %1%")%str_mode);
    }
  }

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewFile(path, mode);
}

ObjectPtr FileCloseFunc::Call(Executor* /*parent*/,
    std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, close)

  FileObject& fs = static_cast<FileObject&>(*params[0]);
  fs.Close();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewNull();
}

ObjectPtr FileReadLineFunc::Call(Executor* /*parent*/,
    std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, read)

  FileObject& file = static_cast<FileObject&>(*params[0]);
  ObjectFactory obj_factory(symbol_table_stack());

  std::string line;
  if(std::getline(file.fs(), line)) {
    return obj_factory.NewString(line);
  }

  return obj_factory.NewBool(false);
}

ObjectPtr FileReadFunc::Call(Executor* /*parent*/,
    std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS_AT_LEAST(params, 1, read)
  SHPP_FUNC_CHECK_NUM_PARAMS_UNTIL(params, 2, read)

  FileObject& file = static_cast<FileObject&>(*params[0]);
  ObjectFactory obj_factory(symbol_table_stack());

  if (params.size() == 2) {
    SHPP_FUNC_CHECK_PARAM_TYPE(params[1], read, INT)
    int len = static_cast<IntObject&>(*params[1]).value();

    if (len < 0) {
      throw RunTimeError(RunTimeError::ErrorCode::INVALID_ARGS,
                         boost::format("invalid length: %1%")%len);
    }

    std::string buffer = file.Read(len);
    return obj_factory.NewString(buffer);
  }

  std::string buffer = file.ReadAll();
  return obj_factory.NewString(buffer);
}

ObjectPtr FileReadAllFunc::Call(Executor* /*parent*/,
    std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, readall)

  FileObject& fs = static_cast<FileObject&>(*params[0]);

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewString(fs.ReadAll());
}

ObjectPtr FileSizeFunc::Call(Executor* /*parent*/,
    std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, size)

  FileObject& fs = static_cast<FileObject&>(*params[0]);

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewInt(fs.Size());
}

ObjectPtr FileWriteFunc::Call(Executor* /*parent*/,
    std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 2, write)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[1], write, STRING)

  FileObject& fs = static_cast<FileObject&>(*params[0]);
  const std::string& str = static_cast<StringObject&>(*params[1]).value();

  fs.Write(str);

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewNull();
}

ObjectPtr FileTellgFunc::Call(Executor* /*parent*/,
    std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, tellg)

  FileObject& fs = static_cast<FileObject&>(*params[0]);

  int pos = fs.Tellg();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewInt(pos);
}

ObjectPtr FileSeekgFunc::Call(Executor* /*parent*/,
    std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS_AT_LEAST(params, 2, seekg)
  SHPP_FUNC_CHECK_NUM_PARAMS_UNTIL(params, 3, seekg)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[1], seekg, INT)

  std::ios_base::seekdir way = std::ios_base::beg;
  int offset = static_cast<IntObject&>(*params[1]).value();

  if (params.size() == 3) {
    SHPP_FUNC_CHECK_PARAM_TYPE(params[2], seekg, INT)
    int iway = static_cast<IntObject&>(*params[2]).value();

    if (iway == 1) {
      way = std::ios_base::cur;
    } else if (iway == 2) {
      way = std::ios_base::end;
    }
  }

  FileObject& fs = static_cast<FileObject&>(*params[0]);
  fs.Seekg(offset, way);

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewNull();
}

}
}
