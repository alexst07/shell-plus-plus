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

#include "file-size-object.h"

#include <sstream>
#include <iomanip>
#include "obj-type.h"
#include "object-factory.h"
#include "utils/check.h"

namespace shpp {
namespace internal {

// Helper to determine the smaller unit between two file sizes
FileSizeObject::Unit FileSizeObject::GetSmallerUnit(Unit u1, Unit u2) {
  // AUTO doesn't participate in comparison, treat it as the larger unit
  if (u1 == Unit::AUTO) return u2;
  if (u2 == Unit::AUTO) return u1;
  
  // Return the smaller unit (lower enum value = smaller unit)
  return (u1 < u2) ? u1 : u2;
}

// FileSizeObject implementation
ObjectPtr FileSizeObject::ObjBool() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(bytes_ != 0);
}

ObjectPtr FileSizeObject::ObjInt() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewInt(static_cast<int>(bytes_));
}

ObjectPtr FileSizeObject::ObjReal() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewReal(static_cast<float>(bytes_));
}

ObjectPtr FileSizeObject::ObjString() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewString(Print());
}

ObjectPtr FileSizeObject::Not() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(bytes_ == 0);
}

ObjectPtr FileSizeObject::Add(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());
  
  if (obj->type() == ObjectType::CUSTON) {
    FileSizeObject* fs_obj = dynamic_cast<FileSizeObject*>(obj.get());
    if (fs_obj) {
      // Use the smaller unit for the result
      Unit result_unit = GetSmallerUnit(unit_, fs_obj->unit_);
      return obj_factory.NewFileSize(bytes_ + fs_obj->bytes_, result_unit);
    }
  }
  
  if (obj->type() == ObjectType::INT) {
    int value = static_cast<IntObject&>(*obj).value();
    // Keep the current unit when adding bytes
    return obj_factory.NewFileSize(bytes_ + value, unit_);
  }
  
  throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                     boost::format("file_size + requires file_size or int"));
}

ObjectPtr FileSizeObject::Sub(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());
  
  if (obj->type() == ObjectType::CUSTON) {
    FileSizeObject* fs_obj = dynamic_cast<FileSizeObject*>(obj.get());
    if (fs_obj) {
      // Use the smaller unit for the result
      Unit result_unit = GetSmallerUnit(unit_, fs_obj->unit_);
      return obj_factory.NewFileSize(bytes_ - fs_obj->bytes_, result_unit);
    }
  }
  
  if (obj->type() == ObjectType::INT) {
    int value = static_cast<IntObject&>(*obj).value();
    // Keep the current unit when subtracting bytes
    return obj_factory.NewFileSize(bytes_ - value, unit_);
  }
  
  throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                     boost::format("file_size - requires file_size or int"));
}

ObjectPtr FileSizeObject::Mult(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());
  
  if (obj->type() == ObjectType::INT) {
    int value = static_cast<IntObject&>(*obj).value();
    // Keep the current unit when multiplying
    return obj_factory.NewFileSize(bytes_ * value, unit_);
  }
  
  if (obj->type() == ObjectType::REAL) {
    float value = static_cast<RealObject&>(*obj).value();
    // Keep the current unit when multiplying
    return obj_factory.NewFileSize(static_cast<long long>(bytes_ * value), unit_);
  }
  
  throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                     boost::format("file_size * requires int or real"));
}

ObjectPtr FileSizeObject::Div(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());
  
  if (obj->type() == ObjectType::INT) {
    int value = static_cast<IntObject&>(*obj).value();
    if (value == 0) {
      throw RunTimeError(RunTimeError::ErrorCode::ZERO_DIV,
                         boost::format("division by zero"));
    }
    // Keep the current unit when dividing
    return obj_factory.NewFileSize(bytes_ / value, unit_);
  }
  
  if (obj->type() == ObjectType::REAL) {
    float value = static_cast<RealObject&>(*obj).value();
    if (value == 0.0f) {
      throw RunTimeError(RunTimeError::ErrorCode::ZERO_DIV,
                         boost::format("division by zero"));
    }
    // Keep the current unit when dividing
    return obj_factory.NewFileSize(static_cast<long long>(bytes_ / value), unit_);
  }
  
  throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                     boost::format("file_size / requires int or real"));
}

ObjectPtr FileSizeObject::Lesser(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());
  
  if (obj->type() == ObjectType::CUSTON) {
    FileSizeObject* fs_obj = dynamic_cast<FileSizeObject*>(obj.get());
    if (fs_obj) {
      return obj_factory.NewBool(bytes_ < fs_obj->bytes_);
    }
  }
  
  if (obj->type() == ObjectType::INT) {
    int value = static_cast<IntObject&>(*obj).value();
    return obj_factory.NewBool(bytes_ < value);
  }
  
  throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                     boost::format("file_size < requires file_size or int"));
}

ObjectPtr FileSizeObject::Greater(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());
  
  if (obj->type() == ObjectType::CUSTON) {
    FileSizeObject* fs_obj = dynamic_cast<FileSizeObject*>(obj.get());
    if (fs_obj) {
      return obj_factory.NewBool(bytes_ > fs_obj->bytes_);
    }
  }
  
  if (obj->type() == ObjectType::INT) {
    int value = static_cast<IntObject&>(*obj).value();
    return obj_factory.NewBool(bytes_ > value);
  }
  
  throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                     boost::format("file_size > requires file_size or int"));
}

ObjectPtr FileSizeObject::LessEqual(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());
  
  if (obj->type() == ObjectType::CUSTON) {
    FileSizeObject* fs_obj = dynamic_cast<FileSizeObject*>(obj.get());
    if (fs_obj) {
      return obj_factory.NewBool(bytes_ <= fs_obj->bytes_);
    }
  }
  
  if (obj->type() == ObjectType::INT) {
    int value = static_cast<IntObject&>(*obj).value();
    return obj_factory.NewBool(bytes_ <= value);
  }
  
  throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                     boost::format("file_size <= requires file_size or int"));
}

ObjectPtr FileSizeObject::GreatEqual(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());
  
  if (obj->type() == ObjectType::CUSTON) {
    FileSizeObject* fs_obj = dynamic_cast<FileSizeObject*>(obj.get());
    if (fs_obj) {
      return obj_factory.NewBool(bytes_ >= fs_obj->bytes_);
    }
  }
  
  if (obj->type() == ObjectType::INT) {
    int value = static_cast<IntObject&>(*obj).value();
    return obj_factory.NewBool(bytes_ >= value);
  }
  
  throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                     boost::format("file_size >= requires file_size or int"));
}

ObjectPtr FileSizeObject::Equal(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());
  
  if (obj->type() == ObjectType::CUSTON) {
    FileSizeObject* fs_obj = dynamic_cast<FileSizeObject*>(obj.get());
    if (fs_obj) {
      return obj_factory.NewBool(bytes_ == fs_obj->bytes_);
    }
  }
  
  if (obj->type() == ObjectType::INT) {
    int value = static_cast<IntObject&>(*obj).value();
    return obj_factory.NewBool(bytes_ == value);
  }
  
  return obj_factory.NewBool(false);
}

ObjectPtr FileSizeObject::NotEqual(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());
  
  if (obj->type() == ObjectType::CUSTON) {
    FileSizeObject* fs_obj = dynamic_cast<FileSizeObject*>(obj.get());
    if (fs_obj) {
      return obj_factory.NewBool(bytes_ != fs_obj->bytes_);
    }
  }
  
  if (obj->type() == ObjectType::INT) {
    int value = static_cast<IntObject&>(*obj).value();
    return obj_factory.NewBool(bytes_ != value);
  }
  
  return obj_factory.NewBool(true);
}

ObjectPtr FileSizeObject::Copy() {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewFileSize(bytes_, unit_);
}

std::string FileSizeObject::Print() {
  std::ostringstream ss;
  
  // If unit is AUTO, use automatic unit selection
  if (unit_ == Unit::AUTO) {
    if (bytes_ >= 1099511627776LL) { // 1 TB
      ss << std::fixed << std::setprecision(2) 
         << (static_cast<double>(bytes_) / 1099511627776.0) << " TB";
    } else if (bytes_ >= 1073741824LL) { // 1 GB
      ss << std::fixed << std::setprecision(2) 
         << (static_cast<double>(bytes_) / 1073741824.0) << " GB";
    } else if (bytes_ >= 1048576LL) { // 1 MB
      ss << std::fixed << std::setprecision(2) 
         << (static_cast<double>(bytes_) / 1048576.0) << " MB";
    } else if (bytes_ >= 1024LL) { // 1 KB
      ss << std::fixed << std::setprecision(2) 
         << (static_cast<double>(bytes_) / 1024.0) << " KB";
    } else {
      ss << bytes_ << " bytes";
    }
  } else {
    // Use the specified unit preference
    switch (unit_) {
      case Unit::BYTES:
        ss << bytes_ << " bytes";
        break;
      case Unit::KB:
        ss << std::fixed << std::setprecision(2) 
           << (static_cast<double>(bytes_) / 1024.0) << " KB";
        break;
      case Unit::MB:
        ss << std::fixed << std::setprecision(2) 
           << (static_cast<double>(bytes_) / 1048576.0) << " MB";
        break;
      case Unit::GB:
        ss << std::fixed << std::setprecision(2) 
           << (static_cast<double>(bytes_) / 1073741824.0) << " GB";
        break;
      case Unit::TB:
        ss << std::fixed << std::setprecision(2) 
           << (static_cast<double>(bytes_) / 1099511627776.0) << " TB";
        break;
      default:
        ss << bytes_ << " bytes";
        break;
    }
  }
  
  return ss.str();
}

ObjectPtr FileSizeObject::Attr(std::shared_ptr<Object> self,
                                const std::string& name) {
  ObjectPtr obj_type = ObjType();
  return static_cast<TypeObject&>(*obj_type).CallObject(name, self);
}

// FileSizeType implementation
FileSizeType::FileSizeType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
    : TypeObject("file_size", obj_type, std::move(sym_table)) {
  // Register static methods
  RegisterStaticMethod<FileSizeBytesFunc>("bytes", symbol_table_stack(), *this);
  RegisterStaticMethod<FileSizeKBFunc>("KB", symbol_table_stack(), *this);
  RegisterStaticMethod<FileSizeMBFunc>("MB", symbol_table_stack(), *this);
  RegisterStaticMethod<FileSizeGBFunc>("GB", symbol_table_stack(), *this);
  RegisterStaticMethod<FileSizeTBFunc>("TB", symbol_table_stack(), *this);
  
  // Register instance methods
  RegisterMethod<FileSizeToBytesFunc>("to_bytes", symbol_table_stack(), *this);
  RegisterMethod<FileSizeToKBFunc>("to_kb", symbol_table_stack(), *this);
  RegisterMethod<FileSizeToMBFunc>("to_mb", symbol_table_stack(), *this);
  RegisterMethod<FileSizeToGBFunc>("to_gb", symbol_table_stack(), *this);
  RegisterMethod<FileSizeToTBFunc>("to_tb", symbol_table_stack(), *this);
}

ObjectPtr FileSizeType::Constructor(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, file_size)
  SHPP_FUNC_CHECK_PARAM_TYPE(params[0], bytes, INT)
  
  int bytes = static_cast<IntObject&>(*params[0]).value();
  
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewFileSize(bytes, FileSizeObject::Unit::BYTES);
}

ObjectPtr FileSizeType::Attr(std::shared_ptr<Object>,
                              const std::string& name) {
  ObjectPtr att_obj = SearchAttr(name);
  return att_obj;
}

// Static method implementations
ObjectPtr FileSizeBytesFunc::Call(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, bytes)
  
  long long bytes = 0;
  if (params[0]->type() == Object::ObjectType::INT) {
    bytes = static_cast<IntObject&>(*params[0]).value();
  } else if (params[0]->type() == Object::ObjectType::REAL) {
    bytes = static_cast<long long>(static_cast<RealObject&>(*params[0]).value());
  } else {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("bytes() requires int or real"));
  }
  
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewFileSize(bytes, FileSizeObject::Unit::BYTES);
}

ObjectPtr FileSizeKBFunc::Call(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, KB)
  
  long long bytes = 0;
  if (params[0]->type() == Object::ObjectType::INT) {
    bytes = static_cast<long long>(static_cast<IntObject&>(*params[0]).value()) * 1024LL;
  } else if (params[0]->type() == Object::ObjectType::REAL) {
    bytes = static_cast<long long>(static_cast<RealObject&>(*params[0]).value() * 1024.0);
  } else {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("KB() requires int or real"));
  }
  
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewFileSize(bytes, FileSizeObject::Unit::KB);
}

ObjectPtr FileSizeMBFunc::Call(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, MB)
  
  long long bytes = 0;
  if (params[0]->type() == Object::ObjectType::INT) {
    bytes = static_cast<long long>(static_cast<IntObject&>(*params[0]).value()) * 1048576LL;
  } else if (params[0]->type() == Object::ObjectType::REAL) {
    bytes = static_cast<long long>(static_cast<RealObject&>(*params[0]).value() * 1048576.0);
  } else {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("MB() requires int or real"));
  }
  
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewFileSize(bytes, FileSizeObject::Unit::MB);
}

ObjectPtr FileSizeGBFunc::Call(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, GB)
  
  long long bytes = 0;
  if (params[0]->type() == Object::ObjectType::INT) {
    bytes = static_cast<long long>(static_cast<IntObject&>(*params[0]).value()) * 1073741824LL;
  } else if (params[0]->type() == Object::ObjectType::REAL) {
    bytes = static_cast<long long>(static_cast<RealObject&>(*params[0]).value() * 1073741824.0);
  } else {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("GB() requires int or real"));
  }
  
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewFileSize(bytes, FileSizeObject::Unit::GB);
}

ObjectPtr FileSizeTBFunc::Call(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, TB)
  
  long long bytes = 0;
  if (params[0]->type() == Object::ObjectType::INT) {
    bytes = static_cast<long long>(static_cast<IntObject&>(*params[0]).value()) * 1099511627776LL;
  } else if (params[0]->type() == Object::ObjectType::REAL) {
    bytes = static_cast<long long>(static_cast<RealObject&>(*params[0]).value() * 1099511627776.0);
  } else {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("TB() requires int or real"));
  }
  
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewFileSize(bytes, FileSizeObject::Unit::TB);
}

// Instance method implementations
ObjectPtr FileSizeToBytesFunc::Call(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, to_bytes)
  
  FileSizeObject& fs = static_cast<FileSizeObject&>(*params[0]);
  
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewInt(static_cast<int>(fs.bytes()));
}

ObjectPtr FileSizeToKBFunc::Call(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, to_kb)
  
  FileSizeObject& fs = static_cast<FileSizeObject&>(*params[0]);
  
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewReal(static_cast<float>(fs.bytes()) / 1024.0f);
}

ObjectPtr FileSizeToMBFunc::Call(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, to_mb)
  
  FileSizeObject& fs = static_cast<FileSizeObject&>(*params[0]);
  
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewReal(static_cast<float>(fs.bytes()) / 1048576.0f);
}

ObjectPtr FileSizeToGBFunc::Call(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, to_gb)
  
  FileSizeObject& fs = static_cast<FileSizeObject&>(*params[0]);
  
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewReal(static_cast<float>(fs.bytes()) / 1073741824.0f);
}

ObjectPtr FileSizeToTBFunc::Call(Executor*, Args&& params, KWArgs&&) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, to_tb)
  
  FileSizeObject& fs = static_cast<FileSizeObject&>(*params[0]);
  
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewReal(static_cast<float>(fs.bytes()) / 1099511627776.0f);
}

}  // namespace internal
}  // namespace shpp

