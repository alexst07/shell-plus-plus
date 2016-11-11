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
    SLICE,
    ARRAY,
    MAP,
    TUPLE,
    FUNC,
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
      return false;
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
      return false;
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
      return false;
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
      return false;
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

class SliceObject: public Object {
 public:
  SliceObject(ObjectPtr obj_start, ObjectPtr obj_end)
      : Object(ObjectType::SLICE) {
    if (obj_start->type() != ObjectType::INT ||
        obj_end->type() != ObjectType::INT) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("slice parameter must be integer"));

      IntObject& int_start = static_cast<IntObject&>(*obj_start);
      IntObject& int_end = static_cast<IntObject&>(*obj_end);

      start_ = int_start.value();
      end_ = int_end.value();
    }
  }

  SliceObject(const SliceObject& obj)
      : Object(obj), start_(obj.start_), end_(obj.end_) {}

  virtual ~SliceObject() {}

  SliceObject& operator=(const SliceObject& obj) {
    start_ = obj.start_;
    end_ = obj.end_;
    return *this;
  }

  inline int start() const noexcept { return start_; }
  inline int end() const noexcept { return start_; }

  std::size_t Hash() const override {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("slice object has no hash method"));
  }

  bool operator==(const Object& obj) const override {
    if (obj.type() != ObjectType::SLICE) {
      return false;
    }

    const SliceObject& slice = static_cast<const SliceObject&>(obj);

    bool exp = (start_ == slice.start_) && (end_ == slice.end_);

    return exp;
  }

  void Print() override {
    std::cout << "SLICE: start = " << start_ << ", end = " << end_;
  }

 private:
  int start_;
  int end_;
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
      : Object(ObjectType::TUPLE), value_(std::move(value)) {}

   TupleObject(const TupleObject& obj): Object(obj), value_(obj.value_) {}

   virtual ~TupleObject() {}

   inline std::shared_ptr<Object>& ElementRef(size_t i) {
     return value_.at(i);
   }

   inline std::shared_ptr<Object> Element(size_t i) {
     return value_.at(i);
   }

   inline size_t Size() const noexcept {
     return value_.size();
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
       return false;
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
       return false;
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
      std::unordered_map<size_t, std::vector<std::pair<ObjectPtr, ObjectPtr>>>;

  using Pair = std::pair<size_t, std::vector<std::pair<ObjectPtr, ObjectPtr>>>;

  MapObject(std::vector<std::pair<ObjectPtr, ObjectPtr>>&& value)
      : Object(ObjectType::MAP) {
    for (auto& e: value) {
      std::vector<std::pair<ObjectPtr, ObjectPtr>> list;
      list.push_back(e);
      value_.insert(std::pair<size_t, std::vector<std::pair<ObjectPtr,
          ObjectPtr>>>(e.first->Hash(), list));
    }
  }

  MapObject(Map&& value)
      : Object(ObjectType::MAP), value_(std::move(value)) {}

  std::size_t Hash() const override {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("map object has no hash method"));
  }

  bool operator==(const Object& obj) const override {
    if (obj.type() != ObjectType::MAP) {
      return false;
    }

    using ls = std::vector<std::pair<ObjectPtr, ObjectPtr>>;
    const MapObject& map = static_cast<const MapObject&>(obj);

    // for to compare two maps
    for (struct {Map::const_iterator a; Map::const_iterator b;} loop
             = { value_.begin(), map.value_.begin() };
         (loop.a != value_.end()) && (loop.b != map.value_.end());
         loop.a++, loop.b++) {
      // for to compare the lists inside the maps
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

  // Return the reference for an object on the map, if there is no
  // entry for this index, create a new empty with this entry and
  // return its reference
  inline ObjectPtr& ElementRef(ObjectPtr obj_index) {
    if (Exists(obj_index)) {
      size_t hash = obj_index->Hash();
      auto it = value_.find(hash);
      return it->second.back().second;
    } else {
      return Insert_(obj_index);
    }
  }

  // Return a tuple object with the element and a bool object
  std::shared_ptr<Object> Element(ObjectPtr obj_index) {
    size_t hash = obj_index->Hash();

    auto it = value_.find(hash);

    // return a tuple with null object and false bool object
    auto ret_null = []() {
      std::vector<std::shared_ptr<Object>> vet_tuple{
          ObjectPtr(ObjectPtr(new NullObject()), new BoolObject(false))};

      ObjectPtr obj_ret(new TupleObject(std::move(vet_tuple)));
      return obj_ret;
    };

    // if the index not exists on the map return a tuple object
    // with null and bool object
    if (it == value_.end()) {
      return ret_null();
    }

    // if the index exists on map, search the object on the list, to confirm
    // that is not a false hash match
    for (auto& e: it->second) {
      // when the obj_index match with any index on the list, create a tuple
      // object to return
      if (*e.first == *obj_index) {
        std::vector<std::shared_ptr<Object>> vet_tuple{
            e.second, ObjectPtr(new BoolObject(true))};

        ObjectPtr obj_ret(new TupleObject(std::move(vet_tuple)));
        return obj_ret;
      } else {
        return ret_null();
      }
    }
  }


  // Create, this method doesn't do any kind of verification
  // the caller method must check if the entry exists on map or not
  inline ObjectPtr& Insert_(ObjectPtr obj_index) {
    size_t hash = obj_index->Hash();

    auto it = value_.find(hash);
    ObjectPtr obj(nullptr);

    // if the hash doesn't exists create a entry with a list
    if (it == value_.end()) {
      std::vector<std::pair<ObjectPtr, ObjectPtr>> list;
      list.push_back(std::pair<ObjectPtr, ObjectPtr>(obj_index, obj));
      value_.insert(Pair(hash, list));
    } else {
      it->second.push_back(std::pair<ObjectPtr, ObjectPtr>(obj_index, obj));
    }

    return value_.find(hash)->second.back().second;
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

  void Print() override {
    std::cout << "MAP: { ";
    for (auto& list: value_) {
      for (auto& pair: list.second) {
        std::cout << "(";
        pair.first->Print();
        std::cout << ", ";
        pair.second->Print();
        std::cout << ")";
      }
    }
    std::cout << "} ";
  }

 private:
   Map value_;
};

class FuncObject: public Object {
 public:
  FuncObject(): Object(ObjectType::FUNC) {}
  virtual ~FuncObject() {}

  std::size_t Hash() const override {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("func object has no hash method"));
  }

  bool operator==(const Object& obj) const override {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("func object has no compare method"));
  }

  virtual ObjectPtr Call(std::vector<ObjectPtr>&& params) = 0;

  void Print() override {
    std::cout << "FUNC";
  }
};

}
}

#endif  // SETI_OBJ_TYPE_H

