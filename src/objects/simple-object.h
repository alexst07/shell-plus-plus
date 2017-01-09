#ifndef SETI_INT_OBJECT_H
#define SETI_INT_OBJECT_H

#include <memory>
#include <iostream>

#include "run_time_error.h"
#include "ast/ast.h"
#include "interpreter/symbol-table.h"
#include "abstract-obj.h"

namespace setti {
namespace internal {

class NullObject: public Object {
 public:
  NullObject(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::NIL, obj_type, std::move(sym_table)) {}

  virtual ~NullObject() {}

  std::size_t Hash() const override {
    throw RunTimeError(RunTimeError::ErrorCode::NULL_ACCESS,
                       boost::format("null object has no hash method"));
  }

  bool operator==(const Object& obj) const override {
    if (obj.type() == ObjectType::NIL) {
      return true;
    }

    return false;
  }

  ObjectPtr ObjBool() override;

  ObjectPtr Equal(ObjectPtr obj) override;

  ObjectPtr NotEqual(ObjectPtr obj) override;

  ObjectPtr And(ObjectPtr) override;

  ObjectPtr Or(ObjectPtr obj) override;

  void Print() override {
    std::cout << "NIL";
  }

  inline std::nullptr_t value() const noexcept { return nullptr; }
};

class BoolObject: public Object {
 public:
  BoolObject(bool value, ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::BOOL, obj_type, std::move(sym_table))
      , value_(value) {}

  BoolObject(const BoolObject& obj): Object(obj), value_(obj.value_) {}

  virtual ~BoolObject() {}

  BoolObject& operator=(const BoolObject& obj) {
    value_ = obj.value_;
    return *this;
  }

  inline bool value() const noexcept { return value_; }

  std::size_t Hash() const override {
    std::hash<bool> bool_hash;
    return bool_hash(value_);
  }

  bool operator==(const Object& obj) const override {
    if (obj.type() != ObjectType::BOOL) {
      return false;
    }

    bool value = static_cast<const BoolObject&>(obj).value_;

    return value_ == value;
  }

  ObjectPtr ObjBool() override;

  ObjectPtr Equal(ObjectPtr obj) override;

  ObjectPtr NotEqual(ObjectPtr obj) override;

  ObjectPtr And(ObjectPtr obj) override;

  ObjectPtr Or(ObjectPtr obj) override;

  void Print() override {
    std::cout << "BOOL: " << value_;
  }

 private:
  bool value_;
};

class IntObject: public Object {
 public:
  IntObject(int value, ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::INT, obj_type, std::move(sym_table))
      , value_(value) {}

  IntObject(const IntObject& obj): Object(obj), value_(obj.value_) {}

  virtual ~IntObject() {}

  IntObject& operator=(const IntObject& obj) {
    value_ = obj.value_;
    return *this;
  }

  inline int value() const noexcept { return value_; }

  std::size_t Hash() const override {
    std::hash<int> int_hash;
    return int_hash(value_);
  }

  ObjectPtr ObjString() override;

  ObjectPtr ObjReal() override;

  bool operator==(const Object& obj) const override {
    if (obj.type() != ObjectType::INT) {
      return false;
    }

    int value = static_cast<const IntObject&>(obj).value_;

    return value_ == value;
  }

  ObjectPtr Add(ObjectPtr obj) override;

  ObjectPtr Sub(ObjectPtr obj) override;

  ObjectPtr Mult(ObjectPtr obj) override;

  ObjectPtr Div(ObjectPtr obj) override;

  ObjectPtr DivMod(ObjectPtr obj) override;

  ObjectPtr RightShift(ObjectPtr obj) override;

  ObjectPtr LeftShift(ObjectPtr obj) override;

  ObjectPtr Lesser(ObjectPtr obj) override;

  ObjectPtr Greater(ObjectPtr obj) override;

  ObjectPtr Copy() override;

  ObjectPtr LessEqual(ObjectPtr obj) override;

  ObjectPtr GreatEqual(ObjectPtr obj) override;

  ObjectPtr Equal(ObjectPtr obj) override;

  ObjectPtr NotEqual(ObjectPtr obj) override;

  ObjectPtr BitAnd(ObjectPtr obj) override;

  ObjectPtr BitOr(ObjectPtr obj) override;

  ObjectPtr BitXor(ObjectPtr obj) override;

  ObjectPtr BitNot() override;

  ObjectPtr UnaryAdd() override;

  ObjectPtr UnarySub() override;

  void Print() override {
    std::cout << "INT: " << value_;
  }

 private:
  ObjectPtr OperationObj(ObjectPtr obj, int op);
  ObjectPtr OperationObjInt(ObjectPtr obj, int op);
  ObjectPtr OperationObjComp(ObjectPtr obj, int op);

  int OperationArit(int a, int b, int op) {
    switch (op) {
      case 0:
        return a + b;
        break;

      case 1:
        return a - b;
        break;

      case 2:
        return a * b;
        break;

      case 3:
        if (b == 0) {
          throw RunTimeError(RunTimeError::ErrorCode::ZERO_DIV,
                             boost::format("zero div indetermined"));
        }

        return a / b;
        break;

      case 4:
        return a % b;
        break;

      case 5:
        return a << b;
        break;

      case 6:
        return a << b;
        break;

      case 7:
        return a & b;
        break;

      case 8:
        return a | b;
        break;

      case 9:
        return a ^ b;
        break;

      default:
        return a;
    }
  }

  float OperationArit(int a, float b, int op) {
    switch (op) {
      case 0:
        return static_cast<float>(a) + b;
        break;

      case 1:
        return static_cast<float>(a) - b;
        break;

      case 2:
        return static_cast<float>(a) * b;
        break;

      case 3:
        if (b == 0) {
          throw RunTimeError(RunTimeError::ErrorCode::ZERO_DIV,
                             boost::format("zero div indetermined"));
        }

        return static_cast<float>(a) / b;
        break;

      default:
        return static_cast<float>(a);
    }
  }

  template<class T>
  bool OperationComp(int a, T b, int op) {
    switch (op) {
      case 0:
        return static_cast<T>(a) < b;
        break;

      case 1:
        return static_cast<T>(a) > b;
        break;

      case 2:
        return static_cast<T>(a) <= b;
        break;

      case 3:
        return static_cast<T>(a) >= b;
        break;

      case 4:
        return static_cast<T>(a) == b;
        break;

      case 5:
        return static_cast<T>(a) != b;
        break;

      default:
        return false;
    }
  }

  int value_;
};

class RealObject: public Object {
 public:
  RealObject(float value, ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::REAL, obj_type, std::move(sym_table))
      , value_(value) {}

  RealObject(const RealObject& obj): Object(obj), value_(obj.value_) {}

  virtual ~RealObject() {}

  RealObject& operator=(const RealObject& obj) {
    value_ = obj.value_;
    return *this;
  }

  inline float value() const noexcept { return value_; }

  ObjectPtr ObjInt() override;

  ObjectPtr ObjString() override;

  std::size_t Hash() const override {
    std::hash<float> float_hash;
    return float_hash(value_);
  }

  bool operator==(const Object& obj) const override {
    if (obj.type() != ObjectType::REAL) {
      return false;
    }

    float value = static_cast<const RealObject&>(obj).value_;

    return value_ == value;
  }

  ObjectPtr OperationObj(ObjectPtr obj, int op);

  ObjectPtr OperationObjComp(ObjectPtr obj, int op);

  ObjectPtr Add(ObjectPtr obj) override;

  ObjectPtr Sub(ObjectPtr obj) override;

  ObjectPtr Mult(ObjectPtr obj) override;

  ObjectPtr Div(ObjectPtr obj) override;

  ObjectPtr Lesser(ObjectPtr obj) override;

  ObjectPtr Greater(ObjectPtr obj) override;

  ObjectPtr Copy() override;

  ObjectPtr LessEqual(ObjectPtr obj) override;

  ObjectPtr GreatEqual(ObjectPtr obj) override;

  ObjectPtr Equal(ObjectPtr obj) override;

  ObjectPtr NotEqual(ObjectPtr obj) override;

  ObjectPtr UnaryAdd() override;

  ObjectPtr UnarySub() override;

  void Print() override {
    std::cout << "REAL: " << value_;
  }

 private:
  float OperationArit(float a, float b, int op) {
    switch (op) {
      case 0:
        return a + b;
        break;

      case 1:
        return a - b;
        break;

      case 2:
        return a * b;
        break;

      case 3:
        if (b == static_cast<float>(0)) {
          throw RunTimeError(RunTimeError::ErrorCode::ZERO_DIV,
                             boost::format("zero div indetermined"));
        }

        return a / b;
        break;

      default:
        return a;
    }
  }

  bool OperationComp(float a, float b, int op) {
    switch (op) {
      case 0:
        return a < b;
        break;

      case 1:
        return a > b;
        break;

      case 2:
        return a <= b;
        break;

      case 3:
        return a >= b;
        break;

      case 4:
        return a == b;
        break;

      case 5:
        return a != b;
        break;

      default:
        return false;
    }
  }

  float value_;
};

}
}

#endif  // SETI_OBJ_TYPE_H
