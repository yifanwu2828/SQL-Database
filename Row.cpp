/**
 * @file Row.cpp
 * @author Yifan Wu
 * @brief 
 * @version 0.9
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "Row.hpp"

#include <iostream>
#include <map>
#include <string>
#include <utility>

#include "BasicTypes.hpp"
#include "Errors.hpp"
#include "Helpers.hpp"

namespace ECE141 {

Row::Row(uint32_t anEntityId, uint32_t aBlockNum)
    : entityId{anEntityId}, blockNum{aBlockNum} {}

Row Row::operator+(const Row &aRow) {
  Row theRow{*this};
  theRow += aRow;
  return theRow;
}

Row &Row::operator+=(const Row &aRow) {
  // std::cerr << "+=copy\n";
  const auto &theTarget = aRow.getData();
  data.insert(theTarget.begin(), theTarget.end());
  return *this;
}

Row &Row::setBlockNum(uint32_t aBlockNum) {
  blockNum = aBlockNum;
  return *this;
}

uint32_t Row::getBlockNum() const { return blockNum; }
KeyValues &Row::getData() { return data; }
const KeyValues &Row::getData() const { return data; }
uint32_t Row::getEntityId() const { return entityId; }

Row &Row::insert(const std::string &aKey, const Value &aValue) {
  // If a key equivalent to k already exists in the container, does nothing.
  // Otherwise, behaves like emplace
  data.try_emplace(aKey, aValue);
  return *this;
}

Row &Row::setAllNull() {
  for (auto &[theKey, theValue] : data) {
    data[theKey] = "NULL";
  }
  return *this;
}

bool Row::setValueForExistingKey(const std::string &aKey, const Value &aValue) {
  if (data.find(aKey) != data.end()) {
    data[aKey] = aValue;
    return true;
  }
  return false;
}

auto Row::encode(std::ostream &anOutput) const -> StatusResult {
  for (const auto &[theAttrName, theVal] : data) {
    if (theAttrName.empty()) {
      continue;
    }
    Helpers::encodeInto(anOutput, theAttrName);
    std::string theEncodeVal = Helpers::replaceWith(
        Helpers::valToString(theVal), whiteSpace, underScore);
    Helpers::encodeInto(anOutput, theEncodeVal);
    // std::cerr << theEncodeVal <<'\n';
    // valToString encode the type at last index
  }
  return {Errors::noError};
}

auto Row::decode(std::istream &anInput) -> StatusResult {
  std::string theAttrName;
  std::string theVal;

  while (!(anInput >> std::ws).eof() && '\0' != anInput.peek()) {
    std::string theDecodeVal;
    anInput >> theAttrName;
    anInput >> theDecodeVal;
    theVal = Helpers::replaceWith(theDecodeVal, underScore, whiteSpace);
    if (!theAttrName.empty() && !theVal.empty()) {
      auto theType = static_cast<DataTypes>(theVal.back());
      theVal.pop_back();
      data.try_emplace(theAttrName, Helpers::toValue(theVal, theType));
    }
  }
  return {Errors::noError};
}

std::ostream &operator<<(std::ostream &aStream, const Row &aRow) {
  aStream << "+-------------------------------------------------------------\n";
  aStream << "| Row \n"
          << "| EntityHash: " << aRow.entityId << '\n'
          << "| blockNum: " << aRow.blockNum << '\n'
          << "+-------------------------------------\n";
  aStream << "| Data:\n| ";
  for (const auto &[theAttrName, theVal] : aRow.data) {
    aStream << '(' << theAttrName << ", " << Helpers::valToString(theVal)
            << "), ";
  }
  aStream
      << "\n+-------------------------------------------------------------\n";
  return aStream;
}
}  // namespace ECE141