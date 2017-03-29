// Copyright 2016 Alex Silva Torres
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "map-object.h"

#include "object-factory.h"
#include "utils/check.h"

namespace shpp {
namespace internal {

MapIterObject::MapIterObject(ObjectPtr map_obj, ObjectPtr obj_type,
                             SymbolTableStack&& sym_table)
    : BaseIter(ObjectType::MAP_ITER, obj_type, std::move(sym_table)) {
  if (map_obj->type() != ObjectType::MAP) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("type is not map"));
  }

  map_obj_ = map_obj;
  pos_ = static_cast<MapObject&>(*map_obj_).value().begin();
  pos_vec_ = 0;
}

ObjectPtr MapIterObject::Equal(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());

  if (obj->type() != ObjectType::MAP_ITER) {
    return obj_factory.NewBool(false);
  }

  MapIterObject& other = static_cast<MapIterObject&>(*obj);

  bool ptr_eq = obj.get() == map_obj_.get();
  bool pos_eq = other.pos_ == pos_;

  return obj_factory.NewBool(ptr_eq && pos_eq);
}

ObjectPtr MapIterObject::Next() {
  ObjectFactory obj_factory(symbol_table_stack());
  MapObject& map_obj = static_cast<MapObject&>(*map_obj_);

  if (pos_ == map_obj.value().end()) {
    ObjectFactory obj_factory(symbol_table_stack());
    return obj_factory.NewNull();
  }

  std::vector<ObjectPtr> vec = {pos_->second[pos_vec_].first,
                                pos_->second[pos_vec_].second};
  pos_vec_++;

  if (pos_vec_ >= pos_->second.size()) {
    pos_++;
    pos_vec_ = 0;
  }

  return obj_factory.NewTuple(std::move(vec));
}

ObjectPtr MapIterObject::HasNext() {
  ObjectFactory obj_factory(symbol_table_stack());
  MapObject& map_obj = static_cast<MapObject&>(*map_obj_);

  if (pos_ == map_obj.value().end()) {
    ObjectFactory obj_factory(symbol_table_stack());
    return obj_factory.NewBool(false);
  }

  return obj_factory.NewBool(true);
}

MapObject::MapObject(std::vector<std::pair<ObjectPtr, ObjectPtr>>&& value,
                     ObjectPtr obj_type, SymbolTableStack&& sym_table)
    : Object(ObjectType::MAP, obj_type, std::move(sym_table)) {
  for (auto& e: value) {
    std::vector<std::pair<ObjectPtr, ObjectPtr>> list;
    list.push_back(e);
    value_.insert(std::pair<size_t, std::vector<std::pair<ObjectPtr,
        ObjectPtr>>>(e.first->Hash(), list));
  }
}

ObjectPtr MapObject::GetItem(ObjectPtr index) {
  return Element(index);
}

ObjectPtr& MapObject::GetItemRef(ObjectPtr index) {
  return ElementRef(index);
}

ObjectPtr MapObject::ObjIter(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewMapIter(obj);
}

ObjectPtr MapObject::Update(ObjectPtr obj, bool override) {
  SHPP_FUNC_CHECK_PARAM_TYPE(obj, add, MAP)

  ObjectFactory obj_factory(symbol_table_stack());
  ObjectPtr map_ptr = obj_factory.NewMap();

  MapObject& new_map = static_cast<MapObject&>(*map_ptr);
  new_map.value_ = value_;

  MapObject& obj_map = static_cast<MapObject&>(*obj);

  // iterate over map to get buckets
  for (auto& bucket: obj_map.value_) {
    // iterate over buckets
    for (auto& pair: bucket.second) {
      if (override) {
        new_map.Insert_(pair.first) = pair.second;
      } else {
        if (!Exists(pair.first)) {
          new_map.Insert_(pair.first) = pair.second;
        }
      }
    }
  }

  return map_ptr;
}

ObjectPtr MapObject::Add(ObjectPtr obj) {
  return Update(obj, false);
}

ObjectPtr& MapObject::ElementRef(ObjectPtr obj_index) {
  if (Exists(obj_index)) {
    size_t hash = obj_index->Hash();
    auto it = value_.find(hash);
    return it->second.back().second;
  } else {
    return Insert_(obj_index);
  }
}

bool MapObject::operator==(const Object& obj) {
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

std::shared_ptr<Object> MapObject::Element(ObjectPtr obj_index) {
  size_t hash = obj_index->Hash();

  auto it = value_.find(hash);

  // return a tuple with null object and false bool object
  auto error = []() {
    throw RunTimeError(RunTimeError::ErrorCode::OUT_OF_RANGE,
                       boost::format("key not found"));
  };

  // if the index not exists on the map return a tuple object
  // with null and bool object
  if (it == value_.end()) {
    error();
  }

  // if the index exists on map, search the object on the list, to confirm
  // that is not a false hash match
  for (auto& e: it->second) {
    // when the obj_index match with any index on the list, return this item
    if (*e.first == *obj_index) {
      return e.second;
    } else {
      error();
    }
  }

  // avoids clang warning
  throw RunTimeError(RunTimeError::ErrorCode::OUT_OF_RANGE,
                     boost::format("key not found"));
}

ObjectPtr& MapObject::Insert_(ObjectPtr obj_index) {
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

bool MapObject::Exists(ObjectPtr obj_index) {
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

std::shared_ptr<Object> MapObject::Attr(std::shared_ptr<Object> self,
                                        const std::string& name) {
  ObjectPtr obj_type = ObjType();
  return static_cast<TypeObject&>(*obj_type).CallObject(name, self);
}

ObjectPtr MapObject::In(ObjectPtr obj) {
  ObjectFactory obj_factory(symbol_table_stack());
  size_t hash = obj->Hash();

  auto it = value_.find(hash);

  if (it != value_.end()) {
    for (auto& e: it->second) {
      ObjectPtr obj_cond = e.first->Equal(obj);
      if (obj_cond->type() != ObjectType::BOOL) {
        throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                           boost::format("equal method must return bool"));
      }

      bool v = static_cast<BoolObject&>(*obj_cond).value();

      if (v) {
        return obj_factory.NewBool(true);
      }
    }
  }

  return obj_factory.NewBool(false);
}

void MapObject::DelItem(ObjectPtr index) {
  size_t hash = index->Hash();

  auto it = value_.find(hash);

  if (it != value_.end()) {
    it->second.erase(std::remove_if(it->second.begin(), it->second.end(),
                     [&index](std::pair<ObjectPtr, ObjectPtr> item) {
      ObjectPtr v_obj = item.first->Equal(index);
      if (v_obj->type() != ObjectType::BOOL) {
        throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                           boost::format("comparation returned not bool"));
      }

      return static_cast<BoolObject&>(*v_obj).value();
    }), it->second.end());
  }
}

long int MapObject::Len() {
  long int size = 0;

  for (auto& v: value_) {
    size += v.second.size();
  }

  return size;
}

MapType::MapType(ObjectPtr obj_type, SymbolTableStack&& sym_table)
    : ContainerType("map", obj_type, std::move(sym_table)) {
  RegisterMethod<MapKeysFunc>("keys", symbol_table_stack(), *this);
  RegisterMethod<MapValuesFunc>("values", symbol_table_stack(), *this);
  RegisterMethod<MapClearFunc>("clear", symbol_table_stack(), *this);
  RegisterMethod<MapUpdateFunc>("update", symbol_table_stack(), *this);
  RegisterMethod<MapExistsFunc>("exists", symbol_table_stack(), *this);
}

ObjectPtr MapKeysFunc::Call(Executor* parent,
                                 std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, keys)

  MapObject& map_obj = static_cast<MapObject&>(*params[0]);

  auto map_elems = map_obj.value();

  std::vector<ObjectPtr> keys;

  for (const auto& map_elem: map_elems) {
    for (const auto& elem: map_elem.second) {
      keys.push_back(elem.first);
    }
  }

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewArray(std::move(keys));
}

ObjectPtr MapValuesFunc::Call(Executor* parent,
                                 std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, values)

  MapObject& map_obj = static_cast<MapObject&>(*params[0]);

  auto map_elems = map_obj.value();

  std::vector<ObjectPtr> keys;

  for (const auto& map_elem: map_elems) {
    for (const auto& elem: map_elem.second) {
      keys.push_back(elem.second);
    }
  }

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewArray(std::move(keys));
}

ObjectPtr MapClearFunc::Call(Executor* parent,
                                 std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 1, clear)

  MapObject& map_obj = static_cast<MapObject&>(*params[0]);

  auto map_elems = map_obj.value();
  map_elems.clear();

  return params[0];
}

ObjectPtr MapUpdateFunc::Call(Executor* parent,
                                 std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS_AT_LEAST(params, 2, update)

  MapObject& map_obj = static_cast<MapObject&>(*params[0]);

  bool override = false;
  if (params.size() == 3) {
    SHPP_FUNC_CHECK_PARAM_TYPE(params[2], override, BOOL)
    override = static_cast<BoolObject&>(*params[2]).value();
  }

  SHPP_FUNC_CHECK_PARAM_TYPE(params[1], map, MAP)

  map_obj.Update(params[1], override);

  return params[0];
}

ObjectPtr MapExistsFunc::Call(Executor* parent,
                                 std::vector<ObjectPtr>&& params) {
  SHPP_FUNC_CHECK_NUM_PARAMS(params, 2, exists)

  MapObject& map_obj = static_cast<MapObject&>(*params[0]);

  bool exists = map_obj.Exists(params[1]);

  ObjectFactory obj_factory(symbol_table_stack());
  return obj_factory.NewBool(exists);
}

}
}
