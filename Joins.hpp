/**
 * @file Joins.hpp
 * @author Yifan Wu
 * @brief 
 * @version 0.9
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef Join_h
#define Join_h

#include <string>
#include <utility>
#include <vector>

#include "Filters.hpp"
#include "keywords.hpp"

namespace ECE141 {

class Join {
 public:
  Join(std::string aTable, Keywords aType)
      : table(std::move(aTable)), joinType(aType) {}

  std::string table;
  Keywords joinType;  // left, right, etc...
  Expressions exprs;
};

using JoinList = std::vector<Join>;

}  // namespace ECE141

#endif /* Join_h */
