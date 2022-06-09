/**
 * @file SQLStatement.hpp
 * @author Yifan Wu
 * @brief
 * @version 0.8
 * @date 2022-04-16
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SQLStatement_hpp
#define SQLStatement_hpp

#include <iosfwd>
#include <memory>
#include <string>

#include "BasicTypes.hpp"  // for StringList
#include "Query.hpp"
#include "Statement.hpp"
#include "keywords.hpp"
#include "Joins.hpp"
#include "Entity.hpp"
#include "Row.hpp"

namespace ECE141 {

class DBProcessor;
class Tokenizer;
class Operand;
class StatusResult;

class SQLStatement : public Statement {
 public:
  explicit SQLStatement(DBProcessor* aDbp,
                        Keywords aKeywordType = Keywords::unknown_kw);

  [[nodiscard]] std::string getStatementName() const override;

 protected:
  DBProcessor* dbp;
};

// ------------------------------------------------------------------------------
// 1. create table {table-name}
class CreateTableStatement : public SQLStatement {
 public:
  explicit CreateTableStatement(DBProcessor* aDbp);
  static bool recognize(Tokenizer& aTokenizer);
  StatusResult parse(Tokenizer& aTokenizer) override;
  StatusResult run(std::ostream& aStream) const override;

 protected:
  std::unique_ptr<Entity> entity{};
};

// ------------------------------------------------------------------------------
// 2. show tables
class ShowTableStatement : public SQLStatement {
 public:
  explicit ShowTableStatement(DBProcessor* aDbp);
  static bool recognize(Tokenizer& aTokenizer);
  StatusResult parse(Tokenizer& aTokenizer) override;
  StatusResult run(std::ostream& aStream) const override;
};

// ------------------------------------------------------------------------------
// 3. drop table {table-name}
class DropTableStatement : public SQLStatement {
 public:
  explicit DropTableStatement(DBProcessor* aDbp);
  static bool recognize(Tokenizer& aTokenizer);
  StatusResult parse(Tokenizer& aTokenizer) override;
  StatusResult run(std::ostream& aStream) const override;
};

// ------------------------------------------------------------------------------
// 4. describe {table-name}
class DescribeTableStatement : public SQLStatement {
 public:
  explicit DescribeTableStatement(DBProcessor* aDbp);
  static bool recognize(Tokenizer& aTokenizer);
  StatusResult parse(Tokenizer& aTokenizer) override;
  StatusResult run(std::ostream& aStream) const override;
};

// ------------------------------------------------------------------------------
// 5. Insert INTO {table-name}
class InsertStatement : public SQLStatement {
 public:
  explicit InsertStatement(DBProcessor* aDbp);
  static bool recognize(Tokenizer& aTokenizer);
  StatusResult parse(Tokenizer& aTokenizer) override;
  StatusResult run(std::ostream& aStream) const override;

 protected:
  RowCollection rows;
};

// ------------------------------------------------------------------------------
// 5. Select * from {table-name}
class SelectStatement : public SQLStatement {
 public:
  explicit SelectStatement(DBProcessor* aDbp,
                           Keywords aType = Keywords::select_kw);

  static bool recognize(Tokenizer& aTokenizer);
  StatusResult parse(Tokenizer& aTokenizer) override;
  StatusResult run(std::ostream& aStream) const override;

  StatusResult parseKeyword(Tokenizer& aTokenizer);
  StatusResult parseTableName(Tokenizer& aTokenizer, std::string& aTableName);
  StatusResult parseFieldList(Tokenizer& aTokenizer);

  StatusResult parseWhereClause(Tokenizer& aTokenizer, const Entity& anEntity);
  StatusResult parseOrderClause(Tokenizer& aTokenizer);
  StatusResult parseLimitClause(Tokenizer& aTokenizer);
  StatusResult parseJoinClause(Tokenizer& aTokenizer);

  StatusResult parseSubField(Tokenizer& aTokenizer, Operand& anOperand);
  StatusResult parseEquation(Tokenizer& aTokenizer, JoinList& aJoins);

 protected:
  DBQuery query;
  const std::string star{"*"};
};

// ------------------------------------------------------------------------------
// 6.Update {table-name} set {attrName} = xxx  where {expression};
class UpdateStatement : public SelectStatement {
 public:
  explicit UpdateStatement(DBProcessor* aDbp);
  static bool recognize(Tokenizer& aTokenizer);
  StatusResult parse(Tokenizer& aTokenizer) override;
  StatusResult run(std::ostream& aStream) const override;
};

// ------------------------------------------------------------------------------
// 7. DELETE FROM {table-name} WHERE where {expression};
class DeleteStatement : public SelectStatement {
 public:
  explicit DeleteStatement(DBProcessor* aDbp);
  static bool recognize(Tokenizer& aTokenizer);
  StatusResult parse(Tokenizer& aTokenizer) override;
  StatusResult run(std::ostream& aStream) const override;
};

class ShowAllIdxStatement : public SelectStatement {
 public:
  explicit ShowAllIdxStatement(DBProcessor* aDbp);
  static bool recognize(Tokenizer& aTokenizer);
  StatusResult parse(Tokenizer& aTokenizer) override;
  StatusResult run(std::ostream& aStream) const override;
};

class ShowIdxStatement : public SelectStatement {
 public:
  explicit ShowIdxStatement(DBProcessor* aDbp);
  static bool recognize(Tokenizer& aTokenizer);
  StatusResult parse(Tokenizer& aTokenizer) override;
  StatusResult run(std::ostream& aStream) const override;

 protected:
  std::string tableName;
  StringList fieldList;
};

}  // namespace ECE141

#endif /* SQLStatement_hpp */
