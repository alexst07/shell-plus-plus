#ifndef SETI_OBJ_TYPE_H
#define SETI_OBJ_TYPE_H

#include <string>
#include <memory>
#include <unordered_map>
#include <tuple>

#include "symbol_table.h"

namespace setti {
namespace internal {

class Object {
 public:
  enum class ObjectType : uint8_t {
    INT,
    REAL,
    STRING,
    ARRAY,
    MAP,
    TUPLE,
    CUSTON
  };

  virtual ~Object() {}

  inline ObjectType type() {
    return type_;
  }

 private:
  ObjectType type_;

 protected:
  Object(ObjectType type): type_(type) {}
};

class IntObject: public Object {
 public:
  IntObject(int value): Object(INT), value_(value) {}
  virtual ~IntObject() {}

  inline int value() const noexcept { return value_; }

 private:
  int value_;
};


}
}

#endif  // SETI_OBJ_TYPE_H

