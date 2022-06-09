/**
 * @file Attribute.hpp
 * @author Yifan Wu, Samuel Woo
 * @brief
 * @version 0.3
 * @date 2022-04-18
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef Attribute_hpp
#define Attribute_hpp

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "BasicTypes.hpp"
#include "Storage.hpp"

namespace ECE141 {

class StatusResult;
/*
  struct Property {
    Property(std::string aName, int aTableId=0) : name(aName),
  tableId(aTableId), desc(true) {} std::string     name; int tableId; bool desc;
  };
  using PropertyList = std::vector<Property>;
*/

/**
Attribute:
  - field_name
  - field_type  (bool, float, integer, timestamp, varchar)  //varchar has length
  - field_length (only applies to text fields)
  - auto_increment (determines if this (integer) field is auto incremented by DB
  - primary_key  (bool indicates that field represents primary key)
  - nullable (bool indicates the field can be null)
**/

class Attribute : Storable {
 public:
  explicit Attribute(DataTypes aType = DataTypes::no_type);
  Attribute(std::string aName, DataTypes aType, uint16_t aSize);

  // setters
  Attribute& setName(const std::string& aName);
  Attribute& setDataType(DataTypes aType);
  Attribute& setSize(int aSize);
  Attribute& setAutoIncrement(bool anAuto);
  Attribute& setPrimaryKey(bool anAuto);
  Attribute& setNullable(bool aNullable);

  Attribute& setDefaultVal(Value aValue);
  Attribute& setDefaultValStr(const std::string& aString);

  bool isValid();  // is this Entity valid?
  bool goodAccess(Value& aVal) const;
  StatusResult isInsertValid(KeyValues& aRowData);

  // getter
  [[nodiscard]] const std::string& getName() const;
  [[nodiscard]] DataTypes getType() const;
  [[nodiscard]] char getCharType() const;
  [[nodiscard]] size_t getSize() const;
  [[nodiscard]] Value getValue() const;
  [[nodiscard]] std::string getValueStr() const;

  [[nodiscard]] bool isPrimaryKey() const;
  [[nodiscard]] bool isNullable() const;
  [[nodiscard]] bool isAutoIncrement() const;
  Value toValue(const std::string& aValue);

  // Storable interface
  StatusResult encode(std::ostream& anOutput) const override;
  StatusResult decode(std::istream& anInput) override;

 protected:
  std::string name;
  DataTypes type{DataTypes::no_type};
  uint16_t size : 10;  // max=1000
  uint16_t autoIncrement : 1;
  uint16_t primary : 1;
  uint16_t nullable : 1;

  // "NULL" indicates variant is empty/has no value
  Value defaultVal;
  std::string defaultValStr{"NULL"};
};

using AttributeOpt = std::optional<Attribute>;
using AttributeList = std::vector<Attribute>;

}  // namespace ECE141

#endif /* Attribute_hpp */
