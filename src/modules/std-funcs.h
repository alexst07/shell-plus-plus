#ifndef SETI_STD_FUNCS_H
#define SETI_STD_FUNCS_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <tuple>

#include "objects/object-factory.h"

namespace seti {
namespace internal {
namespace module {
namespace stdf {

class PrintFunc: public FuncObject {
 public:
  PrintFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);

 private:
  ObjectFactory obj_factory_;
};

class PrintErrFunc: public FuncObject {
 public:
  PrintErrFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);

 private:
  ObjectFactory obj_factory_;
};

class LenFunc: public FuncObject {
 public:
  LenFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);

 private:
  ObjectFactory obj_factory_;
};

class AssertFunc: public FuncObject {
 public:
  AssertFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);

 private:
  ObjectFactory obj_factory_;
};

inline void RegisterModule(SymbolTableStack& sym_table) {
  ModuleCustonObject::MemberTable table = {
    {"print",                 ObjectMethod<PrintFunc>(sym_table)},
    {"print_err",             ObjectMethod<PrintErrFunc>(sym_table)},
    {"len",                   ObjectMethod<LenFunc>(sym_table)},
    {"assert",                ObjectMethod<AssertFunc>(sym_table)}
  };

  for (auto& pair: table) {
    SymbolAttr sym_entry(pair.second, true);
    sym_table.InsertEntry(pair.first, std::move(sym_entry));
  }
}

}
}
}
}

#endif  // SETI_STD_FUNCS_H


