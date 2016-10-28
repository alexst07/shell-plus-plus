#ifndef SETTI_MSG_H
#define SETTI_MSG_H

#include <string>
#include <memory>
#include <unordered_map>
#include <boost/format.hpp>

#include "obj_type.h"

namespace setti {
namespace internal {

class SymbolAttr {
 public:
  SymbolAttr();

 private:
  bool global_;
  std::unique_ptr<Object> value_;

};

class SymbolTable {
 public:
  using SymbolMap = std::unordered_map<std::string, SymbolAttr>;

  SymbolTable();

  bool Insert(std::pair<std::string, SymbolAttr>&& symbol);
  SymbolAttr& Lookup(const std::string& name);
  bool Remove(const std::string& name);

 private:
  SymbolMap map_;
};

class SymbolTableStack {
 public:
  SymbolTableStack();

  void Push(SymbolTable&& table);
  void Pop();
  SymbolAttr& Lookup(const std::string& name);
  bool InsertEntry(std::pair<std::string, SymbolAttr>&& symbol);
};

}
}

#endif  // SETTI_MSG_H

