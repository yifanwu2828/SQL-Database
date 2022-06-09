/**
 * @file Query.hpp
 * @author Yifan Wu
 * @brief
 * @version 0.6
 * @date 2022-04-27
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef DBQuery_hpp
#define DBQuery_hpp

#include <string>
#include <utility>
#include <cstddef>


#include "BasicTypes.hpp"
#include "Filters.hpp"
#include "Entity.hpp"
#include "Joins.hpp"

namespace ECE141 {

class StatusResult;
class Tokenizer;
class DBQuery {
 public:
  DBQuery() = default;
  ~DBQuery() = default;

  DBQuery& setTableName(const std::string& aTableName);

  DBQuery& setUpdateAttrs(const std::string& anAttrName, const Value &aValue);

  DBQuery& setSelectedAttrs(StringList& anAttrNames);

  DBQuery& setOrderBy(StringList& anOrderNames);

  DBQuery& setLimit(size_t aNum);

  DBQuery& setEntity(const Entity& anEntity);
  
  DBQuery& setJoins(JoinList& anEntity);

  [[nodiscard]] std::string getEntityName() const;
  [[nodiscard]] std::pair<std::string, Value> getUpdateKV() const;
  [[nodiscard]] StringList getFieldList() const;
  [[nodiscard]] StringList getOrderBy() const;
  [[nodiscard]] size_t getLimit() const;
  [[nodiscard]] size_t getExpressionNum() const;
  [[nodiscard]] const Filters& getFilter() const;
  [[nodiscard]] const Entity& getEntity() const;
  [[nodiscard]] const JoinList& getJoins() const;

  StatusResult parseExpressions(Tokenizer& aTokenizer, const Entity& anEntity);

 protected:
  /**
   * select (fieldList)
   * FROM (table:ist) JOINS ????
   * WHERE (ExpressionList)
   * GROUP BY (AttrList) -- partioning of our data
   * ORDER BY (view ordering)
   */
  using PairKV = std::pair<std::string, Value>;

  std::string entityName;  // from
  PairKV updateKV;
  StringList selectedAttrNames;  // select
  StringList orderBy;
  size_t limit = 0;
  Filters filter;
  Entity entity;
  JoinList joins;
};

}  // namespace ECE141
#endif /* Query_hpp */