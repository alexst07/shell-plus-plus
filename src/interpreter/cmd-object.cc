#include "cmd-object.h"

#include <string>
#include <boost/variant.hpp>
#include <boost/algorithm/string.hpp>

#include "obj_type.h"
#include "object-factory.h"

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

  if (outerr == 0) {
    std::string str_cmd = cmd_obj_ref.str_stdout();
    boost::trim_if(str_cmd, boost::is_any_of(delim));
    boost::algorithm::split(str_split_, str_cmd, boost::is_any_of(delim),
                            boost::algorithm::token_compress_on);
  } else {
    std::string str_cmd = cmd_obj_ref.str_stderr();
    boost::trim_if(str_cmd, boost::is_any_of(delim));
    boost::algorithm::split(str_split_, str_cmd, boost::is_any_of(delim),
                            boost::algorithm::token_compress_on);
  }
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
  return obj_factory.NewCmdIter("\n", 0, obj);
}


}
}