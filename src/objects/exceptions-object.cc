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

#include "regex.h"

#include "obj-type.h"
#include "object-factory.h"
#include "utils/check.h"
#include "run_time_error.h"

namespace shpp {
namespace internal {

#define DEFINE_EXCEPTION(NAME)                                                \
  NAME ## Object::NAME ## Object(const std::string& msg, ObjectPtr obj_type,  \
      SymbolTableStack&& sym_table)                                           \
      : Object(ObjectType::REGEX, obj_type, std::move(sym_table))             \
      , msg_(msg) {                                                           \
  }                                                                           \
                                                                              \
  ObjectPtr NAME ## Object::ObjString() {                                     \
    ObjectFactory obj_factory(symbol_table_stack());                          \
    return obj_factory.NewString(msg_);                                       \
  }                                                                           \
                                                                              \
  ObjectPtr NAME ## Object::Attr(std::shared_ptr<Object> self,                \
                              const std::string& name) {                      \
    ObjectPtr obj_type = ObjType();                                           \
    return static_cast<TypeObject&>(*obj_type).CallObject(name, self);        \
  }                                                                           \
                                                                              \
  NAME ## Type::NAME ## Type(ObjectPtr obj_type,                              \
      SymbolTableStack&& sym_table, ObjectPtr base)                           \
      : TypeObject(#NAME, obj_type, std::move(sym_table), base,               \
                   TypeObject::InterfacesList(), ObjectType::TYPE, false) {   \
    RegisterMethod<NAME ## InitFunc>("__init__", symbol_table_stack(), *this);\
    RegisterMethod<NAME ## StrFunc>("__str__", symbol_table_stack(), *this);  \
  }                                                                           \
                                                                              \
  std::shared_ptr<Object> NAME ## Type::Attr(std::shared_ptr<Object>,         \
      const std::string& name) {                                              \
    ObjectPtr att_obj = SearchAttr(name);                                     \
    return att_obj;                                                           \
  }                                                                           \
                                                                              \
  ObjectPtr NAME ## Type::Constructor(Executor*, Args&& params, KWArgs&&) {   \
    SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, NAME)                               \
    SHPP_FUNC_CHECK_PARAM_TYPE(params[0], NAME, STRING)                       \
                                                                              \
    const std::string& str = static_cast<StringObject&>(*params[0]).value();  \
                                                                              \
    ObjectFactory obj_factory(symbol_table_stack());                          \
    return obj_factory.New ## NAME(str);                                      \
  }                                                                           \
                                                                              \
ObjectPtr NAME ## InitFunc::Call(Executor*, Args&& params, KWArgs&&) {        \
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 2, __init__)                             \
  SHPP_FUNC_CHECK_PARAM_TYPE(params[1], __init__, STRING)                     \
                                                                              \
  params[0]->symbol_table_stack().SetEntry("msg_", params[1]);                \
                                                                              \
  return params[0];                                                           \
}                                                                             \
                                                                              \
ObjectPtr NAME ## StrFunc::Call(Executor*, Args&& params, KWArgs&&) {         \
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, __str__)                              \
                                                                              \
  return params[0]->symbol_table_stack().Lookup("msg_", false).SharedAccess();\
}

DEFINE_EXCEPTION(Exception)
DEFINE_EXCEPTION(NullAccessException)
DEFINE_EXCEPTION(LookupException)
DEFINE_EXCEPTION(InvalidCmdException)
DEFINE_EXCEPTION(BadAllocException)
DEFINE_EXCEPTION(IndexException)
DEFINE_EXCEPTION(KeyException)
DEFINE_EXCEPTION(InvalidArgsException)
DEFINE_EXCEPTION(TypeException)
DEFINE_EXCEPTION(FuncParamsException)
DEFINE_EXCEPTION(ZeroDivException)
DEFINE_EXCEPTION(FdNotFoundException)
DEFINE_EXCEPTION(IOException)
DEFINE_EXCEPTION(ImportException)
DEFINE_EXCEPTION(AssertException)
DEFINE_EXCEPTION(ParserException)
DEFINE_EXCEPTION(RegexException)
DEFINE_EXCEPTION(GlobException)
DEFINE_EXCEPTION(EvalException)
DEFINE_EXCEPTION(ErrorException)

ObjectPtr MapExceptionError(RunTimeError& err, SymbolTableStack& sym_table) {
  ObjectFactory obj_factory(sym_table);

  switch (err.err_code()) {
    case RunTimeError::ErrorCode::NULL_ACCESS:
      return obj_factory.NewNullAccessException(err.msg());
      break;

    case RunTimeError::ErrorCode::SYMBOL_NOT_FOUND:
      return obj_factory.NewLookupException(err.msg());
      break;

    case RunTimeError::ErrorCode::BAD_ALLOC:
      return obj_factory.NewBadAllocException(err.msg());
      break;

    case RunTimeError::ErrorCode::OUT_OF_RANGE:
      return obj_factory.NewIndexException(err.msg());
      break;

    case RunTimeError::ErrorCode::KEY_NOT_FOUND:
      return obj_factory.NewKeyException(err.msg());
      break;

    case RunTimeError::ErrorCode::ID_NOT_FOUND:
      return obj_factory.NewKeyException(err.msg());
      break;

    case RunTimeError::ErrorCode::INCOMPATIBLE_TYPE:
      return obj_factory.NewTypeException(err.msg());
      break;

    case RunTimeError::ErrorCode::INVALID_ARGS:
      return obj_factory.NewInvalidArgsException(err.msg());
      break;

    case RunTimeError::ErrorCode::FUNC_PARAMS:
      return obj_factory.NewFuncParamsException(err.msg());
      break;

    case RunTimeError::ErrorCode::ZERO_DIV:
      return obj_factory.NewZeroDivException(err.msg());
      break;

    case RunTimeError::ErrorCode::FD_NOT_FOUND:
      return obj_factory.NewFdNotFoundException(err.msg());
      break;

    case RunTimeError::ErrorCode::INVALID_OPCODE:
      return obj_factory.NewErrorException(err.msg());
      break;

    case RunTimeError::ErrorCode::FILE:
      return obj_factory.NewIOException(err.msg());
      break;

    case RunTimeError::ErrorCode::IMPORT:
      return obj_factory.NewImportException(err.msg());
      break;

    case RunTimeError::ErrorCode::INVALID_COMMAND:
      return obj_factory.NewInvalidCmdException(err.msg());
      break;

    case RunTimeError::ErrorCode::ASSERT:
      return obj_factory.NewAssertException(err.msg());
      break;

    case RunTimeError::ErrorCode::PARSER:
      return obj_factory.NewParserException(err.msg());
      break;

    case RunTimeError::ErrorCode::REGEX:
      return obj_factory.NewRegexException(err.msg());
      break;

    case RunTimeError::ErrorCode::GLOB:
      return obj_factory.NewGlobException(err.msg());
      break;

    case RunTimeError::ErrorCode::EVAL:
      return obj_factory.NewEvalException(err.msg());
      break;

    default:
      return obj_factory.NewErrorException(err.msg());
  }
}

}
}
