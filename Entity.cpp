/**
 * @file Entity.cpp
 * @author Yifan Wu, Samuel Woo
 * @brief
 * @version 0.4
 * @date 2022-04-18
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "Entity.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "Attribute.hpp"
#include "Errors.hpp"
#include "Helpers.hpp"

namespace ECE141 {
Entity::Entity(std::string aName) : name{std::move(aName)} {}

Entity &Entity::setName(std::string &aName) {
  name = std::move(aName);
  return *this;
}

// ---------------------------------------------
// Storable interface
StatusResult Entity::encode(std::ostream &anOutput) const {
  Helpers::encodeInto(anOutput, name);
  Helpers::encodeInto(anOutput, autoincr);
  for (const auto &attr : attributes) {
    attr.encode(anOutput);
  }
  return {Errors::noError};
}

StatusResult Entity::decode(std::istream &anInput) {
  if (!(anInput >> std::ws).eof()) {
    Helpers::decodeFrom(anInput, name);
    Helpers::decodeFrom(anInput, autoincr);
    while (!(anInput >> std::ws).eof() && '\0' != anInput.peek()) {
      Attribute theAttr;
      theAttr.decode(anInput);
      if (!Helpers::checkStreamNotFail(anInput)) {
        break;
      }
      attributes.emplace_back(std::move(theAttr));
    }
  }
  return {Errors::noError};
}
// ---------------------------------------------

Entity &Entity::addAttribute(const Attribute &anAttribute) {
  // do not add duplicate attribute
  if (nullptr == getAttribute(anAttribute.getName())) {
    attributes.emplace_back(anAttribute);
  }
  return *this;
}

Attribute *Entity::getAttribute(const std::string &aName) const {
  auto it =
      std::find_if(attributes.begin(), attributes.end(),
                   [&](const Attribute &x) { return x.getName() == aName; });
  return (it != attributes.end()) ? const_cast<Attribute *>(&(*it)) : nullptr;
}

// USE: ask the entity for name of primary key (may not have one...)
const Attribute *Entity::getPrimaryKey() const {
  auto it = std::find_if(attributes.begin(), attributes.end(),
                         [&](const Attribute &x) { return x.isPrimaryKey(); });
  return (it != attributes.end()) ? &(*it) : nullptr;
}

}  // namespace ECE141
