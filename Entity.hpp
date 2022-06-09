/**
 * @file Entity.hpp
 * @author Yifan Wu
 * @brief 
 * @version 0.9
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef Entity_hpp
#define Entity_hpp

#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

#include "Attribute.hpp"
#include "Storage.hpp"

namespace ECE141 {

class StatusResult;
using AttributeOpt = std::optional<Attribute>;
using AttributeList = std::vector<Attribute>;

//------------------------------------------------

// The Entity class is an in-memory representation of a table defintion.
class Entity : Storable {
 public:
  explicit Entity(std::string aName = "");

  Entity& setName(std::string& aName);
  [[nodiscard]] const std::string& getName() const { return name; }
  [[nodiscard]] const AttributeList& getAttributes() const {
    return attributes;
  }
  [[nodiscard]] Attribute* getAttribute(const std::string& aName) const;
  [[nodiscard]] const Attribute* getPrimaryKey() const;
  Entity& addAttribute(const Attribute& anAttribute);
  [[nodiscard]] int getAutoIncrID() { return autoincr++; }

  // ---------------------------------------------
  // Storable interface
  StatusResult encode(std::ostream& anOutput) const override;
  StatusResult decode(std::istream& anInput) override;
  // ---------------------------------------------

 protected:
  std::string name;
  int autoincr{1};  // start from 1
  AttributeList attributes;
};

}  // namespace ECE141
#endif /* Entity_hpp */
