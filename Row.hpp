/**
 * @file Row.hpp
 * @author Yifan Wu
 * @brief 
 * @version 0.9
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef Row_hpp
#define Row_hpp

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <iosfwd>

// #include "Attribute.hpp"
#include "BasicTypes.hpp"
#include "Storage.hpp"

namespace ECE141 {
class StatusResult;

class Row : Storable {
 public:
  Row(uint32_t anEntityId = 0, uint32_t aBlockNum = 0);

  Row operator+(const Row& aRow);
  Row& operator+=(const Row& aRow);

  Row& setBlockNum(uint32_t aBlockNum);
  [[nodiscard]] uint32_t getBlockNum() const;

  Row& insert(const std::string& aKey, const Value& aValue);
  bool setValueForExistingKey(const std::string& aKey, const Value& aValue);
  Row& setAllNull();
  KeyValues& getData();
  uint32_t getEntityId() const;

  [[nodiscard]] const KeyValues& getData() const;
  // Storable interface
  StatusResult encode(std::ostream& anOutput) const override;
  StatusResult decode(std::istream& anInput) override;

 protected:
  uint32_t entityId{0};  // hash value of entity
  uint32_t blockNum{0};  // ! BlockNum is default to 0.
  KeyValues data;
  std::string whiteSpace{" "};
  std::string underScore{"#_#"};

  friend std::ostream& operator<<(std::ostream& aStream, const Row& aRow);
};

//-------------------------------------------

using RowCollection = std::vector<std::unique_ptr<Row>>;

}  // namespace ECE141
#endif /* Row_hpp */

