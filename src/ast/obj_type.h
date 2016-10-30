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
    BOOL,
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
  IntObject(int value): Object(ObjectType::INT), value_(value) {}
  virtual ~IntObject() {}

  inline int value() const noexcept { return value_; }

 private:
  int value_;
};

class BoolObject: public Object {
 public:
  BoolObject(bool value): Object(ObjectType::BOOL), value_(value) {}
  virtual ~BoolObject() {}

  inline bool value() const noexcept { return value_; }

 private:
  bool value_;
};

class RealObject: public Object {
 public:
  RealObject(float value): Object(ObjectType::REAL), value_(value) {}
  virtual ~RealObject() {}

  inline float value() const noexcept { return value_; }

 private:
  float value_;
};

class StringObject: public Object {
 public:
  StringObject(std::string value): Object(ObjectType::STRING), value_(value) {}
  virtual ~StringObject() {}

  inline const std::string& value() const noexcept { return value_; }

 private:
  std::string value_;
};

}
}

#endif  // SETI_OBJ_TYPE_H

