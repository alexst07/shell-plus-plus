#include "cmd-object.h"

#include <string>
#include <boost/variant.hpp>
#include <boost/algorithm/string.hpp>

#include "obj-type.h"
#include "object-factory.h"
#include "utils/check.h"

namespace setti {
namespace internal {

CmdIterObject::CmdIterObject(std::string delim, int outerr, ObjectPtr cmd_obj,
                             ObjectPtr obj_type, SymbolTableStack&& sym_table)
    : Object(ObjectType::CMD_ITER, obj_type, std::move(sym_table))
    , pos_(0) {
  if (cmd_obj->type() != ObjectType::CMD) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("only cmdobj supported"));
  }

  cmd_obj_ = cmd_obj;
  CmdObject& cmd_obj_ref = static_cast<CmdObject&>(*cmd_obj_);

  std::string str_cmd;

  if (outerr == 0) {
    str_cmd = cmd_obj_ref.str_stdout();
  } else {
    str_cmd = cmd_obj_ref.str_stderr();
  }

  boost::trim_if(str_cmd, boost::is_any_of(delim));

  if (str_cmd.empty()) {
    return;
  }

  boost::algorithm::split(str_split_, str_cmd, boost::is_any_of(delim),
                          boost::algorithm::token_compress_on);
}

ObjectPtr CmdIterObject::Next() {
  std::string str = str_split_.at(pos_++);

  ObjectFactory obj_factory(symbol_table_stack());
  ObjectPtr obj_str(obj_factory.NewString(std::move(str)));

  return obj_str;
}

ObjectPtr CmdIterObject::HasNext() {
  ObjectFactory obj_factory(symbol_table_stack());

  bool v = pos_ == str_split_.size();
  return obj_factory.NewBool(!v);
}

ObjectPtr CmdObject::ObjIter(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewCmdIter(delim_, 0, obj);
}

ObjectPtr CmdObject::ObjString()  {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewString(str_stdout());
}

std::shared_ptr<Object> CmdObject::Attr(std::shared_ptr<Object> self,
                                        const std::string& name) {
  ObjectPtr obj_type = ObjType();
  return static_cast<TypeObject&>(*obj_type).CallObject(name, self);
}

ObjectPtr CmdType::Constructor(Executor* /*parent*/,
                               std::vector<ObjectPtr>&& /*params*/) {
  throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,
                     boost::format("cmdobj is not constructable"));
}

ObjectPtr CmdOutFunc::Call(Executor* /*parent*/,
                           std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS(params, 1, out)

  CmdObject& cmd_obj = static_cast<CmdObject&>(*params[0]);

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewString(cmd_obj.str_stdout());
}

ObjectPtr CmdDelimFunc::Call(Executor* /*parent*/,
                           std::vector<ObjectPtr>&& params) {
  SETI_FUNC_CHECK_NUM_PARAMS_UNTIL(params, 2, delim)

  CmdObject& cmd_obj = static_cast<CmdObject&>(*params[0]);

  if (params.size() == 2) {
    SETI_FUNC_CHECK_PARAM_TYPE(params[1], delim, STRING)
    std::string delim = static_cast<StringObject&>(*params[1]).value();
    cmd_obj.set_delim(delim);
    return params[0];
  }

  std::string delim = cmd_obj.delim();

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewString(delim);
}

}
}
