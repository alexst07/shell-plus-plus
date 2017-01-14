#ifndef SETI_STD_FUNCS_H
#define SETI_STD_FUNCS_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <tuple>

#include "objects/object-factory.h"

namespace setti {
namespace internal {
namespace module {
namespace stdf {

// Temporary declaration of functions
class PrintFunc: public FuncObject {
 public:
  PrintFunc(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , obj_factory_(symbol_table_stack()) {}

  ObjectPtr Call(Executor* /*parent*/, std::vector<ObjectPtr>&& params);

 private:
  ObjectFactory obj_factory_;
};

inline void RegisterModule(SymbolTableStack& sym_table) {
  ModuleCustonObject::MemberTable table = {
    {"print",                 ObjectMethod<PrintFunc>(sym_table)}
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


