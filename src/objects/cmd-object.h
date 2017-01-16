#ifndef SETI_CMD_OBJECT_H
#define SETI_CMD_OBJECT_H

#include <memory>
#include <iostream>
#include <vector>

#include "run_time_error.h"
#include "ast/ast.h"
#include "interpreter/symbol-table.h"
#include "abstract-obj.h"
#include "obj-type.h"
#include "func-object.h"

namespace setti {
namespace internal {

class CmdIterObject: public Object {
 public:
  CmdIterObject(std::string delim, int outerr, ObjectPtr cmd_obj,
                ObjectPtr obj_type, SymbolTableStack&& sym_table);

  virtual ~CmdIterObject() {}

  ObjectPtr Next() override;

  ObjectPtr HasNext() override;

  std::string Print() override {
    return std::string("[cmd_iter]");
  }

 private:
  size_t pos_;
  ObjectPtr cmd_obj_;
  std::vector<std::string> str_split_;
};

class CmdObject: public Object {
 public:
   CmdObject(int status, std::string&& str_stdout, std::string&& str_stderr,
             ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::CMD, obj_type, std::move(sym_table))
      , status_(status)
      , str_stdout_(std::move(str_stdout))
      , str_stderr_(std::move(str_stderr))
      , delim_("\n") {}

   virtual ~CmdObject() {}

   ObjectPtr ObjIter(ObjectPtr obj) override;

   const std::string& str_stdout() const noexcept {
     return str_stdout_;
   }

   const std::string& str_stderr() const noexcept {
     return str_stderr_;
   }

   ObjectPtr ObjString() override;

   std::shared_ptr<Object> Attr(std::shared_ptr<Object> self,
                                const std::string& name) override;

   std::string Print() override {
     return str_stdout_;
   }

   long int Len() override {
     return str_stdout_.size();
   }

   inline void set_delim(const std::string& delim) {
     delim_ = delim;
   }

   const std::string delim() {
     return delim_;
   }

 private:
  int status_;
  std::string str_stdout_;
  std::string str_stderr_;
  std::string delim_;
};

class CmdType: public TypeObject {
 public:
  CmdType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : TypeObject("cmdobj", obj_type, std::move(sym_table)) {}

  virtual ~CmdType() {}

  ObjectPtr Constructor(Executor*, std::vector<ObjectPtr>&&) override;
};

class CmdOutFunc: public FuncObject {
 public:
  CmdOutFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);
};

class CmdDelimFunc: public FuncObject {
 public:
  CmdDelimFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table)) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);
};

}
}

#endif  // SETI_CMD_OBJECT_H
