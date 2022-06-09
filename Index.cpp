/**
 * @file Index.cpp
 * @author Yifan Wu
 * @brief 
 * @version 0.9
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "Index.hpp"

#include <iostream>
#include <utility>
#include <optional>

#include "Errors.hpp"
#include "Helpers.hpp"
#include "BlockIO.hpp"

namespace ECE141 {

using ValueProxy = Index::ValueProxy;

// ValueProxy interface -----------------------------------------
ValueProxy::ValueProxy(Index &anIndex, uint32_t aKey)
    : index(anIndex), key(aKey), type(IndexType::intKey) {}

ValueProxy::ValueProxy(Index &anIndex, const std::string &aKey)
    : index(anIndex), key(aKey), type(IndexType::strKey) {}

ValueProxy &ValueProxy::operator=(uint32_t aValue) {
  index.setKeyValue(key, aValue);
  return *this;
}

ValueProxy::operator IntOpt() const { return index.valueAt(key); }
// ------------------------------------------------------------

// Index interface
Index::Index(Storage &aStorage, uint32_t aBlockNum, IndexType aType,
             const std::string &aName)
    : storage{aStorage}, name{aName}, blockNum{aBlockNum}, type{aType} {
  entityId = ("Null" != name) ? Helpers::hashString(aName) : 0;
}

ValueProxy Index::operator[](const std::string &aKey) {
  return {*this, aKey};
  // return ValueProxy(*this, aKey);
}

ValueProxy Index::operator[](uint32_t aKey) {
  return {*this, aKey};
  // return ValueProxy(*this, aKey);
}

Index &Index::setChanged(bool aChanged) {
  changed |= aChanged;  // changed = aChanged;
  return *this;
}

Index &Index::setIndexBlockNum(uint32_t aBlockNum) {
  blockNum = aBlockNum;
  return *this;
}

// add key / value
bool Index::setKeyValue(const IndexKey &aKey, uint32_t aValue) {
  data.emplace(aKey, aValue);
  // data[aKey]=aValue;
  return changed = true;  // side-effect intended!
}

bool Index::isChanged() const { return changed; }

uint32_t Index::getBlockNum() const { return blockNum; }
std::string Index::getName() const { return name; }
uint32_t Index::getEntityId() const { return entityId; }
size_t Index::getSize() const { return data.size(); }
IndexType Index::getType() const {return type;}

StorageInfo Index::getStorageInfo(size_t aSize,
                                  const std::string &aTableName) const {
  return {entityId, aSize, static_cast<int32_t>(blockNum),
          BlockType::index_block, aTableName + '.' + name};
}

bool Index::isEmpty() const { return data.empty(); }

// find value
bool Index::exists(const IndexKey &aKey) const {
  return data.find(aKey) != data.end();
}

// get value
IntOpt Index::valueAt(const IndexKey &aKey) const {
  return exists(aKey) ? data.at(aKey) : (IntOpt)(std::nullopt);
}

// remove key / value
StatusResult Index::erase(const std::string &aKey) {
  auto it = data.find(aKey);
  if (it != data.end()) {
    data.erase(it);
    setChanged(true);
  } else {
    std::cerr << "Key: '" << aKey << "' NOT FOUND!\n";
  }
  return {Errors::noError};
}

StatusResult Index::erase(uint32_t aKey) {
  auto it = data.find(aKey);
  if (it != data.end()) {
    data.erase(it);
    setChanged(true);
  } else {
    std::cerr << "Key: " << aKey << " NOT FOUND!\n";
  }
  return {Errors::noError};
}

// Storable interface --------------------------------
StatusResult Index::encode(std::ostream &anOutput) const {
  Helpers::encodeInto(anOutput, name);
  Helpers::encodeInto(anOutput, static_cast<char>(type));
  Helpers::encodeInto(anOutput, blockNum);
  if (!data.empty()) {
    for (const auto &[theIndexKey, theBlockNum] : data) {
      encodeIndexKey(anOutput, theIndexKey, type);
      Helpers::encodeInto(anOutput, static_cast<uint32_t>(theBlockNum));
    }
  }
  return {Errors::noError};
}

StatusResult Index::decode(std::istream &anInput) {
  // Erases all elements from the map
  data.clear();
  if (!(anInput >> std::ws).eof()) {
    Helpers::decodeFrom(anInput, name);
    entityId = Helpers::hashString(name);

    char theTypeIdx;
    Helpers::decodeFrom(anInput, theTypeIdx);
    type = static_cast<IndexType>(theTypeIdx);
    Helpers::decodeFrom(anInput, blockNum);

    while (!(anInput >> std::ws).eof() && '\0' != anInput.peek()) {
      std::string theIndexKeyStr;
      Helpers::decodeFrom(anInput, theIndexKeyStr);
      uint32_t theBlockNum{0};
      Helpers::decodeFrom(anInput, theBlockNum);
      if (0 != theBlockNum) {
        setKeyValue(toIndexKey(theIndexKeyStr, type), theBlockNum);
      }
    }
  }
  return {Errors::noError};
}
// --------------------------------------------------

// visit blocks associated with index
bool Index::each(BlockVisitor aVisitor) {
  Block theBlock;
  for (const auto &[theIndexKey, theBlockNum] : data) {
    //if (storage.readBlock(theBlockNum, theBlock)) {
      if (!aVisitor(theBlock, theBlockNum)) {
        return false;
      }
    //}
  }
  return true;
}

// for show
// visit index values (key, value)...
bool Index::eachKV(const IndexVisitor &aCall) {
  for (const auto &[theIndexKey, theBlockNum] : data) {
    if (!aCall(theIndexKey, theBlockNum)) {
      return false;
    }
  }
  return true;
}

Index &Index::setName(std::string aName) {
  entityId = Helpers::hashString(aName);
  name = std::move(aName);
  return *this;
}

void Index::encodeIndexKey(std::ostream &anOutput, const IndexKey &anIndexKey,
                           const IndexType &anIdxType) {
  if (IndexType::intKey == anIdxType) {
    Helpers::encodeInto(anOutput, std::get<uint32_t>(anIndexKey));
  } else {
    Helpers::encodeInto(anOutput, std::get<std::string>(anIndexKey));
  }
}

IndexKey Index::toIndexKey(const std::string &aStr, IndexType anIdxType) {
  IndexKey theKey;
  if (IndexType::intKey == anIdxType) {
    theKey = static_cast<uint32_t>(std::stoul(aStr));
  } else if (IndexType::strKey == anIdxType) {
    theKey = aStr;
  }
  return theKey;
}

}  // namespace ECE141
