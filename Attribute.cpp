/**
 * @file Attribute.cpp
 * @author Yifan Wu, Samuel Woo
 * @brief
 * @version 0.6
 * @date 2022-04-18
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "Attribute.hpp"

#include <utility>
#include <variant>
#include "Errors.hpp"
#include "Helpers.hpp"

namespace ECE141 {

Attribute::Attribute(DataTypes aType)
    : type{aType}, size{0}, autoIncrement{0}, primary{0}, nullable{1} {}

Attribute::Attribute(std::string aName, DataTypes aType, uint16_t aSize = 0)
    : name{std::move(aName)},
      type{aType},
      size{aSize},
      autoIncrement{0},
      primary{0},
      nullable{1} {}

Attribute& Attribute::setName(const std::string& aName) {
  name = aName;
  return *this;
}

Attribute& Attribute::setDataType(DataTypes aType) {
  type = aType;
  return *this;
}

Attribute& Attribute::setSize(int aSize) {
  size = aSize;
  return *this;
}

Attribute& Attribute::setAutoIncrement(bool anAuto) {
  autoIncrement = anAuto;
  return *this;
}

Attribute& Attribute::setPrimaryKey(bool aPrimary) {
  primary = aPrimary;
  return *this;
}

Attribute& Attribute::setNullable(bool aNullable) {
  nullable = aNullable;
  return *this;
}

Attribute& Attribute::setDefaultVal(Value aValue) {
  defaultVal = std::move(aValue);
  return *this;
}

Attribute& Attribute::setDefaultValStr(const std::string& aString) {
  defaultValStr = aString;
  return *this;
}

const std::string& Attribute::getName() const { return name; }
DataTypes Attribute::getType() const { return type; }
char Attribute::getCharType() const { return static_cast<char>(type); }
size_t Attribute::getSize() const { return size; }
Value Attribute::getValue() const { return defaultVal; }
std::string Attribute::getValueStr() const { return defaultValStr; }
bool Attribute::isPrimaryKey() const { return primary; }
bool Attribute::isNullable() const { return nullable; }
bool Attribute::isAutoIncrement() const { return autoIncrement; }

bool Attribute::goodAccess(Value& aVal) const {
  try {
    switch (getType()) {
      case DataTypes::bool_type:
        std::get<bool>(aVal);
        break;
      case DataTypes::float_type:
        std::get<double>(aVal);
        break;
      case DataTypes::int_type:
        std::get<int>(aVal);
        break;
      case DataTypes::varchar_type:
        std::get<std::string>(aVal);
        break;
      case DataTypes::datetime_type:
        std::get<std::string>(aVal);
        break;
      default:
        return false;
    }
  } catch (std::bad_variant_access const& ex) {
    return false;
  }
  return true;
}

// TODO:  do validation on attribute
bool Attribute::isValid() { return true; }

auto Attribute::isInsertValid(KeyValues& aRowData) -> StatusResult {
  if (!goodAccess(aRowData[name])) {
    return {Errors::invalidAttribute};
  }
  return {Errors::noError};
}

auto Attribute::toValue(const std::string& aValue) -> Value {
  setDefaultValStr(aValue);

  //  the last return can handle this case
  // if (DataTypes::datetime_type == type) {
  //     return aValue;
  // }
  if (Helpers::isNumber<int>(aValue) || Helpers::isNumber<double>(aValue)) {
    if (DataTypes::int_type == type) {
      return stoi(aValue);
    }
    if (DataTypes::float_type == type) {
      return stod(aValue);
    }
  } else if (aValue == "false") {
    return false;
  } else if (aValue == "true") {
    return true;
  } else if (aValue == "NULL") {
    return aValue;
  }
  return aValue;
}

// Storable interface
auto Attribute::encode(std::ostream& anOutput) const -> StatusResult {
  Helpers::encodeInto(anOutput, name);
  Helpers::encodeInto(anOutput, static_cast<char>(type));
  Helpers::encodeInto(anOutput, size);
  Helpers::encodeInto(anOutput, autoIncrement);
  Helpers::encodeInto(anOutput, primary);
  Helpers::encodeInto(anOutput, nullable);
  Helpers::encodeInto(anOutput, defaultValStr);
  return {Errors::noError};
}

auto Attribute::decode(std::istream& anInput) -> StatusResult {
  char theCharType;
  int theSize;
  bool theAutoIncr;
  bool thePrim;
  bool theNull;
  Helpers::decodeFrom(anInput, name);
  Helpers::decodeFrom(anInput, theCharType);
  Helpers::decodeFrom(anInput, theSize);
  Helpers::decodeFrom(anInput, theAutoIncr);
  Helpers::decodeFrom(anInput, thePrim);
  Helpers::decodeFrom(anInput, theNull);
  Helpers::decodeFrom(anInput, defaultValStr);

  setDataType(static_cast<DataTypes>(theCharType));
  setSize(theSize);
  setAutoIncrement(theAutoIncr);
  setPrimaryKey(thePrim);
  setNullable(theNull);
  setDefaultVal(toValue(defaultValStr));
  return {Errors::noError};
}
}  // namespace ECE141
