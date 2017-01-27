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

namespace seti {
namespace internal {

MapIterObject::MapIterObject(ObjectPtr map_obj, ObjectPtr obj_type,
                             SymbolTableStack&& sym_table)
    : Object(ObjectType::MAP_ITER, obj_type, std::move(sym_table)) {
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

ObjectPtr& MapObject::ElementRef(ObjectPtr obj_index) {
  if (Exists(obj_index)) {
    size_t hash = obj_index->Hash();
    auto it = value_.find(hash);
    return it->second.back().second;
  } else {
    return Insert_(obj_index);
  }
}

bool MapObject::operator==(const Object& obj) const {
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

long int MapObject::Len() {
  long int size = 0;

  for (auto& v: value_) {
    size += v.second.size();
  }

  return size;
}

}
}
