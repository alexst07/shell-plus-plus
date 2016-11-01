#ifndef SETI_OBJ_TYPE_H
#define SETI_OBJ_TYPE_H

#include <string>
#include <memory>
#include <unordered_map>
#include <tuple>

#include "symbol_table.h"

namespace setti {
namespace internal {

class Object: public LeftPointer<Object> {
 public:
  enum class ObjectType: uint8_t {
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

  LeftPointer::EntryType entry_type() const noexcept {
    return LeftPointer::EntryType::OBJECT;
  }

 private:
  ObjectType type_;

 protected:
  Object(ObjectType type): LeftPointer(*this), type_(type) {}
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
  StringObject(std::string&& value)
      : Object(ObjectType::STRING), value_(std::move(value)) {}
  virtual ~StringObject() {}

  inline const std::string& value() const noexcept { return value_; }

 private:
  std::string value_;
};

class ArrayObject: public Object {
 public:
   ArrayObject(std::vector<std::unique_ptr<Object>> value)
      : Object(ObjectType::ARRAY), value_(std::move(value)) {}
   virtual ~ArrayObject() {}

   inline Object* at(size_t i) {
     return value_.at(i).get();
   }

   inline std::unique_ptr<Object>& ElementRef(size_t i) {
     return value_.at(i);
   }

   inline void set(size_t i, std::unique_ptr<Object> obj) {
     value_[i] = std::move(obj);
   }

 private:
  std::vector<std::unique_ptr<Object>> value_;
};

}
}

#endif  // SETI_OBJ_TYPE_H

