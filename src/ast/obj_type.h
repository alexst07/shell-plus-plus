#ifndef SETI_OBJ_TYPE_H
#define SETI_OBJ_TYPE_H

#include <string>
#include <memory>
#include <unordered_map>
#include <tuple>
#include <list>

#include "run_time_error.h"

namespace setti {
namespace internal {

class EntryPointer {
 public:
  enum class EntryType: uint8_t {
    SYMBOL,
    OBJECT
  };

  EntryType entry_type() const noexcept {
    return type_;
  }

  EntryPointer(const EntryPointer& other): type_(other.type_) {}

 protected:
  EntryPointer(EntryType type): type_(type) {}

 private:
  EntryType type_;
};

class Object: public EntryPointer {
 public:
  enum class ObjectType: uint8_t {
    NIL,
    INT,
    BOOL,
    REAL,
    STRING,
    ARRAY,
    MAP,
    TUPLE,
    CUSTON
  };

  Object(const Object& obj): EntryPointer(obj), type_(obj.type_) {}

  virtual ~Object() {}

  inline ObjectType type() const {
    return type_;
  }

  virtual void Print() = 0;

  virtual std::size_t Hash() const = 0;

  virtual bool operator==(const Object& obj) const = 0;

 private:
  ObjectType type_;

 protected:
  Object(ObjectType type)
      : EntryPointer(EntryPointer::EntryType::OBJECT), type_(type) {}
};

typedef std::shared_ptr<Object> ObjectPtr;

class NullObject: public Object {
 public:
  NullObject(): Object(ObjectType::NIL) {}
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

  void Print() override {
    std::cout << "NIL";
  }

  inline nullptr_t value() const noexcept { return nullptr; }
};

class IntObject: public Object {
 public:
  IntObject(int value): Object(ObjectType::INT), value_(value) {}
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

  bool operator==(const Object& obj) const override {
    if (obj.type() != ObjectType::INT) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("operator == valid only with int"));
    }

    int value = static_cast<const IntObject&>(obj).value_;

    return value_ == value;
  }

  void Print() override {
    std::cout << "INT: " << value_;
  }

 private:
  int value_;
};

class BoolObject: public Object {
 public:
  BoolObject(bool value): Object(ObjectType::BOOL), value_(value) {}
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
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("operator == valid only with bool"));
    }

    bool value = static_cast<const BoolObject&>(obj).value_;

    return value_ == value;
  }

  void Print() override {
    std::cout << "BOOL: " << value_;
  }

 private:
  bool value_;
};

class RealObject: public Object {
 public:
  RealObject(float value): Object(ObjectType::REAL), value_(value) {}
  RealObject(const RealObject& obj): Object(obj), value_(obj.value_) {}
  virtual ~RealObject() {}

  RealObject& operator=(const RealObject& obj) {
    value_ = obj.value_;
    return *this;
  }

  inline float value() const noexcept { return value_; }

  std::size_t Hash() const override {
    std::hash<float> float_hash;
    return float_hash(value_);
  }

  bool operator==(const Object& obj) const override {
    if (obj.type() != ObjectType::REAL) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("operator == valid only with real"));
    }

    float value = static_cast<const RealObject&>(obj).value_;

    return value_ == value;
  }

  void Print() override {
    std::cout << "REAL: " << value_;
  }

 private:
  float value_;
};

class StringObject: public Object {
 public:
  StringObject(std::string&& value)
      : Object(ObjectType::STRING), value_(std::move(value)) {}
  StringObject(const StringObject& obj): Object(obj), value_(obj.value_) {}

  virtual ~StringObject() {}

  StringObject& operator=(const StringObject& obj) {
    value_ = obj.value_;
    return *this;
  }

  inline const std::string& value() const noexcept { return value_; }

  std::size_t Hash() const override {
    std::hash<std::string> str_hash;
    return str_hash(value_);
  }

  bool operator==(const Object& obj) const override {
    if (obj.type() != ObjectType::STRING) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("operator == valid only with string"));
    }

    std::string value = static_cast<const StringObject&>(obj).value_;

    return value_ == value;
  }

  void Print() override {
    std::cout << "STRING: " << value_;
  }

 private:
  std::string value_;
};
class TupleObject: public Object {
 public:
   TupleObject(std::vector<std::unique_ptr<Object>>&& value)
      : Object(ObjectType::TUPLE), value_(value.size()) {
     for (size_t i = 0; i < value.size(); i++) {
       Object* obj_ptr = value[i].release();
       value_[i] = std::shared_ptr<Object>(obj_ptr);
     }
   }

   TupleObject(std::vector<std::shared_ptr<Object>>&& value)
      : Object(ObjectType::TUPLE), value_(value) {}

   TupleObject(const TupleObject& obj): Object(obj), value_(obj.value_) {}

   virtual ~TupleObject() {}

   inline std::shared_ptr<Object>& ElementRef(size_t i) {
     return value_.at(i);
   }

   inline std::shared_ptr<Object> Element(size_t i) {
     return value_.at(i);
   }

   inline void set(size_t i, std::unique_ptr<Object> obj) {
     Object* obj_ptr = obj.release();
     value_[i] = std::shared_ptr<Object>(obj_ptr);
   }

   std::size_t Hash() const override {
     if (value_.empty()) {
       throw RunTimeError(RunTimeError::ErrorCode::OUT_OF_RANGE,
                          boost::format("hash of empty tuple is not valid"));
     }

     size_t hash = 0;

     // Executes xor operation with hash of each element of tuple
     for (auto& e: value_) {
       hash ^= e->Hash();
     }

     return hash;
   }

   bool operator==(const Object& obj) const override {
     if (obj.type() != ObjectType::TUPLE) {
       throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                          boost::format("operator == valid only with tuple"));
     }

     const TupleObject& tuple_obj = static_cast<const TupleObject&>(obj);

     // If the tuples have different size, they are different
     if (tuple_obj.value_.size() != value_.size()) {
       return false;
     }

     bool r = true;

     // Test each element on tuple
     for (size_t i = 0; i < value_.size(); i++) {
       r = r && (tuple_obj.value_[i] == value_[i]);
     }

     return r;
   }

   void Print() override {
     std::cout << "TUPLE: ( ";
     for (const auto& e: value_) {
       e->Print();
       std::cout << " ";
     }
     std::cout << ")";
   }

 private:
  std::vector<std::shared_ptr<Object>> value_;
};

class ArrayObject: public Object {
 public:
   ArrayObject(std::vector<std::unique_ptr<Object>>&& value)
      : Object(ObjectType::ARRAY), value_(value.size()) {
     for (size_t i = 0; i < value.size(); i++) {
       Object* obj_ptr = value[i].release();
       value_[i] = std::shared_ptr<Object>(obj_ptr);
     }
   }

   ArrayObject(std::vector<std::shared_ptr<Object>>&& value)
      : Object(ObjectType::ARRAY), value_(value) {}

   ArrayObject(const ArrayObject& obj): Object(obj), value_(obj.value_) {}

   virtual ~ArrayObject() {}

   inline Object* at(size_t i) {
     return value_.at(i).get();
   }

   inline std::shared_ptr<Object>& ElementRef(size_t i) {
     return value_.at(i);
   }

   inline std::shared_ptr<Object> Element(size_t i) {
     return value_.at(i);
   }

   inline void set(size_t i, std::unique_ptr<Object> obj) {
     Object* obj_ptr = obj.release();
     value_[i] = std::shared_ptr<Object>(obj_ptr);
   }

   std::size_t Hash() const override {
     if (value_.empty()) {
       throw RunTimeError(RunTimeError::ErrorCode::OUT_OF_RANGE,
                          boost::format("hash of empty array is not valid"));
     }

     size_t hash = 0;

     // Executes xor operation with hash of each element of array
     for (auto& e: value_) {
       hash ^= e->Hash();
     }

     return hash;
   }

   bool operator==(const Object& obj) const override {
     if (obj.type() != ObjectType::ARRAY) {
       throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                          boost::format("operator == valid only with array"));
     }

     const ArrayObject& array_obj = static_cast<const ArrayObject&>(obj);

     // If the tuples have different size, they are different
     if (array_obj.value_.size() != value_.size()) {
       return false;
     }

     bool r = true;

     // Test each element on tuple
     for (size_t i = 0; i < value_.size(); i++) {
       r = r && (array_obj.value_[i] == value_[i]);
     }

     return r;
   }

   void Print() override {
     std::cout << "ARRAY: [ ";
     for (const auto& e: value_) {
       e->Print();
       std::cout << " ";
     }
     std::cout << "]";
   }

 private:
  std::vector<std::shared_ptr<Object>> value_;
};

class MapObject: public Object {
 public:
  using Map =
      std::unordered_map<size_t, std::list<std::pair<ObjectPtr, ObjectPtr>>>;

  using Pair = std::pair<size_t, std::list<std::pair<ObjectPtr, ObjectPtr>>>;

  MapObject(std::vector<std::pair<ObjectPtr, ObjectPtr>>&& value)
      : Object(ObjectType::MAP) {
    for (size_t i = 0; i < value.size(); i++) {
      // max efficiency inserting
      auto it = value_.begin();
      std::list<std::pair<ObjectPtr, ObjectPtr>> list;
      list.push_back(std::move(value[i]));
      value_.insert(it, Pair(value[i].first->Hash(), std::move(list)));
    }
  }

  MapObject(Map&& value)
      : Object(ObjectType::MAP), value_(std::move(value)) {}

  std::size_t Hash() const override {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("map object has no hash method"));
  }

  bool operator==(const Object& obj) const override {
    using ls = std::list<std::pair<ObjectPtr, ObjectPtr>>;
    const MapObject& map = static_cast<const MapObject&>(obj);

    for (struct {Map::const_iterator a; Map::const_iterator b;} loop
             = { value_.begin(), map.value_.begin() };
         (loop.a != value_.end()) && (loop.b != map.value_.end());
         loop.a++, loop.b++) {
      for (struct {ls::const_iterator la; ls::const_iterator lb;} l
               = { loop.a->second.begin(), loop.b->second.begin() };
           (l.la != loop.a->second.end()) && (l.lb != loop.b->second.end());
           l.la++, l.lb++) {
        if (*l.la != *l.lb) {
          return false;
        }
      }
    }

    return true;
  }

  inline std::shared_ptr<Object>& ElementRef(size_t i) {
    return value_.at(i);
  }

  // Return a tuple object with the element and a bool object
  std::shared_ptr<Object> Element(ObjectPtr obj_index) {
    // if the index not exists on the map return a tuple object
    // with null and bool object
    if (!Exists(obj_index)) {
      std::vector<std::shared_ptr<Object>> vet_tuple{
          ObjectPtr(ObjectPtr(new NullObject()), new BoolObject(false))};

      ObjectPtr obj_ret(New TupleObject(std::move(vet_tuple)));
      return obj_ret;
    }


    // if the index exists on map, search the object on the list, to confirm
    // that is not a false hash match

  }

  inline bool Exists(ObjectPtr obj_index) {
    size_t hash = obj_index->Hash();

    auto it = value_.find(hash);

    if (it != value_.end()) {
      for (auto& e: it->second) {
        if (*e.first == *obj_index) {
          return true;
        }
      }
    }

    return false;
  }

  virtual void Print() = 0;

 private:
   Map value_;
};

}
}

#endif  // SETI_OBJ_TYPE_H

