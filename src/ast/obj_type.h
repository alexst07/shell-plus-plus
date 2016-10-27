#ifndef SETI_OBJ_TYPE_H
#define SETI_OBJ_TYPE_H

#include <string>
#include <memory>
#include <unordered_map>
#include <tuple>

#include "symbol_table.h"

namespace setti {
namespace internal {

class ObjType {
};

class Method {

};

class ObjTypeNative: public ObjType {
 public:

  std::tuple<bool> GetMethod(const std::string& name);
 private:
  SymbolTable sym_table_;
};


}
}

#endif  // SETI_OBJ_TYPE_H

