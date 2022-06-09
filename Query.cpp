/**
 * @file Query.cpp
 * @author Yifan Wu
 * @brief
 * @version 0.6
 * @date 2022-04-29
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "Query.hpp"

#include <string>
#include <utility>

#include "Errors.hpp"
// #include "Tokenizer.hpp"

namespace ECE141 {

DBQuery& DBQuery::setTableName(const std::string& aTableName) {
  entityName = aTableName;
  return *this;
}

DBQuery& DBQuery::setSelectedAttrs(StringList& anAttrNames) {
  selectedAttrNames = std::move(anAttrNames);
  return *this;
}

DBQuery& DBQuery::setUpdateAttrs(const std::string& anAttrName,
                                 const Value& aValue) {
  updateKV = {anAttrName, aValue};
  return *this;
}

DBQuery& DBQuery::setOrderBy(StringList& anOrderNames) {
  orderBy = std::move(anOrderNames);
  return *this;
}

DBQuery& DBQuery::setLimit(size_t aNum) {
  limit = aNum;
  return *this;
}

DBQuery& DBQuery::setEntity(const Entity& anEntity) {
  entity = anEntity;
  return *this;
}

DBQuery& DBQuery::setJoins(JoinList& aJoinList) {
  joins = std::move(aJoinList);
  return *this;
}

auto DBQuery::getEntityName() const -> std::string { return entityName; }
auto DBQuery::getUpdateKV() const -> std::pair<std::string, Value> {
  return updateKV;
}

auto DBQuery::getFieldList() const -> StringList { return selectedAttrNames; }
auto DBQuery::getOrderBy() const -> StringList { return orderBy; }

auto DBQuery::getLimit() const -> size_t { return limit; }
auto DBQuery::getExpressionNum() const -> size_t {
  return filter.getExpressionsNum();
}

const Entity& DBQuery::getEntity() const { return entity; }
const Filters& DBQuery::getFilter() const { return filter; }
const JoinList& DBQuery::getJoins() const { return joins; }

auto DBQuery::parseExpressions(Tokenizer& aTokenizer, const Entity& anEntity)
    -> StatusResult {
  return filter.parse(aTokenizer, anEntity);
}

}  // namespace ECE141
