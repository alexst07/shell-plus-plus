#ifndef SETI_CMD_OBJECT_H
#define SETI_CMD_OBJECT_H

#include <memory>
#include <iostream>
#include <vector>

#include "run_time_error.h"
#include "ast/ast.h"
#include "symbol_table.h"
#include "abstract-obj.h"

namespace setti {
namespace internal {

class CmdIterObject: public Object {
 public:
  CmdIterObject(std::string delim, int outerr, ObjectPtr cmd_obj,
                ObjectPtr obj_type, SymbolTableStack&& sym_table);

  virtual ~CmdIterObject() {}

  ObjectPtr Next() override;

  ObjectPtr HasNext() override;

  void Print() override {
    std::cout << "CMD ITER";
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
      , str_stderr_(std::move(str_stderr)){}

   virtual ~CmdObject() {}

   ObjectPtr ObjIter(ObjectPtr obj) override;

   const std::string& str_stdout() const noexcept {
     return str_stdout_;
   }

   const std::string& str_stderr() const noexcept {
     return str_stderr_;
   }

   void Print() override {
     std::cout << "CMD-OBJ";
   }

 private:
  std::string str_stdout_;
  std::string str_stderr_;
  int status_;
};

}
}

#endif  // SETI_CMD_OBJECT_H
