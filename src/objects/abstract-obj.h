#ifndef SETI_ABSTRACT_OBJ_H
#define SETI_ABSTRACT_OBJ_H

#include <string>
#include <memory>
#include <unordered_map>
#include <tuple>
#include <list>

#include "run_time_error.h"
#include "interpreter/symbol-table.h"

namespace setti {
namespace internal {

class Executor;

class Object {
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
    CMD,
    TYPE,
    ARRAY_ITER,
    MAP_ITER,
    TUPLE_ITER,
    CMD_ITER,
    DECL_TYPE,
    DECL_OBJ,
    MODULE,
    CUSTON
  };

  virtual ~Object() {}

  inline ObjectType type() const {
    return type_;
  }

  virtual std::string Print() = 0;

  virtual long int Len() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no len interface"));
  }

  virtual std::size_t Hash() const {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no hash interface"));
  }

  virtual bool operator==(const Object& obj) const {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no print method"));
  }

  virtual std::shared_ptr<Object> ObjBool() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no bool interface"));
  }

  virtual std::shared_ptr<Object> ObjString() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no string interface"));
  }

  virtual std::shared_ptr<Object> ObjInt() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no int interface"));
  }

  virtual std::shared_ptr<Object> ObjReal() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no real interface"));
  }

  virtual std::shared_ptr<Object> ObjCmd() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no cmd interface"));
  }

  virtual std::shared_ptr<Object> GetItem(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no get_item interface"));
  }

  virtual std::shared_ptr<Object>& GetItemRef(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no get_item interface"));
  }

  // This method must receive the self object to apply the iterator
  virtual std::shared_ptr<Object> ObjIter(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no iter interface"));
  }

  virtual std::shared_ptr<Object> Add(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no + operator"));
  }

  virtual std::shared_ptr<Object> Sub(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no - operator"));
  }

  virtual std::shared_ptr<Object> Mult(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no * operator"));
  }

  virtual std::shared_ptr<Object> Div(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no / operator"));
  }

  virtual std::shared_ptr<Object> DivMod(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no % operator"));
  }

  virtual std::shared_ptr<Object> RightShift(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no >> operator"));
  }

  virtual std::shared_ptr<Object> LeftShift(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no << operator"));
  }

  virtual std::shared_ptr<Object> Lesser(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no < operator"));
  }

  virtual std::shared_ptr<Object> Greater(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no > operator"));
  }

  virtual std::shared_ptr<Object> Copy() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no copy method"));
  }

  virtual std::shared_ptr<Object> Next() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no next method"));
  }

  virtual std::shared_ptr<Object> HasNext() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no has_next method"));
  }

  virtual std::shared_ptr<Object> LessEqual(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no >= operator"));
  }

  virtual std::shared_ptr<Object> GreatEqual(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no <= operator"));
  }

  virtual std::shared_ptr<Object> Equal(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no == operator"));
  }

  virtual std::shared_ptr<Object> NotEqual(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no == operator"));
  }

  virtual std::shared_ptr<Object> BitAnd(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no & operator"));
  }

  virtual std::shared_ptr<Object> BitOr(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no | operator"));
  }

  virtual std::shared_ptr<Object> BitXor(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no ^ operator"));
  }

  virtual std::shared_ptr<Object> BitNot() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no ~ operator"));
  }

  virtual std::shared_ptr<Object> And(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no && operator"));
  }

  virtual std::shared_ptr<Object> Or(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no && operator"));
  }

  virtual std::shared_ptr<Object> UnaryAdd() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no unary + operator"));
  }

  virtual std::shared_ptr<Object> UnarySub() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no unary - operator"));
  }

  virtual std::shared_ptr<Object> Not() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no not operator"));
  }

  virtual std::shared_ptr<Object> Begin() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no begin method"));
  }

  virtual std::shared_ptr<Object> End() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no end method"));
  }

  virtual std::shared_ptr<Object> Attr(std::shared_ptr<Object>,
                                        const std::string&) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no attr method"));
  }

  // this method must be used when arrow operation is on left side
  virtual std::shared_ptr<Object>& AttrAssign(std::shared_ptr<Object>,
                                        const std::string&) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type has no attr method"));
  }

  std::shared_ptr<Object> ObjType() const noexcept {
    return obj_type_.lock();
  }

 private:
  // enum type
  ObjectType type_;

  // type of object, it is other object
  std::weak_ptr<Object> obj_type_;

  SymbolTableStack sym_table_;

 protected:
  Object(ObjectType type, std::shared_ptr<Object> obj_type,
         SymbolTableStack&& sym_table)
      : type_(type)
      , obj_type_(obj_type)
      , sym_table_(std::move(sym_table)){}

  SymbolTableStack& symbol_table_stack() {
    return sym_table_;
  }
};

typedef std::shared_ptr<Object> ObjectPtr;

}
}

#endif  // SETI_ABSTRACT_OBJ_H
