/**
 * @file SQLStatement.cpp
 * @author Yifan Wu, Samuel Woo
 * @brief
 * @version 0.6
 * @date 2022-04-17
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "SQLStatement.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <vector>

#include "Attribute.hpp"
#include "BasicTypes.hpp"
#include "DBProcessor.hpp"
#include "Entity.hpp"
#include "Errors.hpp"
#include "Filters.hpp"
#include "Helpers.hpp"
#include "ParseHelper.hpp"
#include "Row.hpp"
#include "SQLProcessor.hpp"
#include "Scanner.hpp"  // for semicolon, left_paren, comma, dot, rig...
#include "TokenSequencer.hpp"
#include "Tokenizer.hpp"

namespace ECE141 {
SQLStatement::SQLStatement(DBProcessor *aDbp, Keywords aKeywordType)
    : Statement{aKeywordType, StatementType::SQL_Statement}, dbp{aDbp} {}

std::string SQLStatement::getStatementName() const {
  static std::map<Keywords, std::string> tableTypeMap{
      {Keywords::create_kw, "Create table Statement"},
      {Keywords::drop_kw, "Drop table Statement"},
      {Keywords::show_kw, "Show tables Statement"},
      {Keywords::describe_kw, "Describe table Statement"},
      {Keywords::insert_kw, "Insert Into table Statement"},
      {Keywords::select_kw, "Select Rows Statement"},
      {Keywords::update_kw, "Update Rows Statement"},
      {Keywords::delete_kw, "Delete Rows Statement"},
  };
  if (tableTypeMap.find(kwType) != tableTypeMap.end()) {
    return tableTypeMap.at(kwType);
  }
  return "SQL Table Statement";
}

// ---------------------------------------------------------------------------
// * CREATE TABLE {table-name}
CreateTableStatement::CreateTableStatement(DBProcessor *aDbp)
    : SQLStatement{aDbp, Keywords::create_kw} {}

bool CreateTableStatement::recognize(Tokenizer &aTokenizer) {
  TokenSequencer theSeq(aTokenizer);
  return theSeq.currentIsNoSkip({Keywords::create_kw, Keywords::table_kw});
}

StatusResult CreateTableStatement::parse(Tokenizer &aTokenizer) {
  StatusResult theResult;
  TokenSequencer theSeq(aTokenizer);

  if (theSeq.currentIs({Keywords::create_kw, Keywords::table_kw})) {
    Token &theToken = aTokenizer.current();
    if (TokenType::identifier == aTokenizer.current().type) {
      std::string &theEntityName = theToken.data;
      entity = std::make_unique<Entity>(theEntityName);
      aTokenizer.next();
      if (aTokenizer.skipIf(left_paren)) {
        while (theResult && aTokenizer.more()) {
          if (TokenType::identifier == aTokenizer.current().type) {
            Attribute theAttr;
            theAttr.setName(aTokenizer.current().data);
            aTokenizer.next();
            ParseHelper theParseHelper{aTokenizer};
            theResult = theParseHelper.parseAttribute(theAttr);
            entity->addAttribute(theAttr);
            aTokenizer.skipIf(semicolon);
          }
        }
      }
    }
  }
  return theResult;
}

StatusResult CreateTableStatement::run(
    [[maybe_unused]] std::ostream &anOutput) const {
  return dbp->createTable(entity.get());
}

// ---------------------------------------------------------------------------
// * SHOW TABLES;
ShowTableStatement::ShowTableStatement(DBProcessor *aDbp)
    : SQLStatement{aDbp, Keywords::show_kw} {}

bool ShowTableStatement::recognize(Tokenizer &aTokenizer) {
  TokenSequencer theSeq(aTokenizer);
  return theSeq.currentIsNoSkip({Keywords::show_kw, Keywords::tables_kw});
}

StatusResult ShowTableStatement::parse(Tokenizer &aTokenizer) {
  aTokenizer.next(2);
  return {Errors::noError};
}

StatusResult ShowTableStatement::run(
    [[maybe_unused]] std::ostream &anOutput) const {
  return dbp->showTables();
}

// ---------------------------------------------------------------------------
// * DROP TABLE {table-name};
DropTableStatement::DropTableStatement(DBProcessor *aDbp)
    : SQLStatement{aDbp, Keywords::drop_kw} {}

bool DropTableStatement::recognize(Tokenizer &aTokenizer) {
  TokenSequencer theSeq(aTokenizer);
  return theSeq.currentIsNoSkip({Keywords::drop_kw, Keywords::table_kw});
}

StatusResult DropTableStatement::parse(Tokenizer &aTokenizer) {
  ParseHelper theParseHelper{aTokenizer};
  return theParseHelper.parseThird(identifierData);
}

StatusResult DropTableStatement::run(
    [[maybe_unused]] std::ostream &anOutput) const {
  return dbp->dropTable(identifierData);
}

// ---------------------------------------------------------------------------
// * DESCRIBE {table-name}
DescribeTableStatement::DescribeTableStatement(DBProcessor *aDbp)
    : SQLStatement{aDbp, Keywords::describe_kw} {}

bool DescribeTableStatement::recognize(Tokenizer &aTokenizer) {
  TokenSequencer theSeq(aTokenizer);
  return theSeq.currentIsNoSkip({Keywords::describe_kw});
}

StatusResult DescribeTableStatement::parse(Tokenizer &aTokenizer) {
  ParseHelper theParseHelper{aTokenizer};
  return theParseHelper.parseSecond(identifierData);
}

StatusResult DescribeTableStatement::run(
    [[maybe_unused]] std::ostream &anOutput) const {
  return dbp->describeTable(identifierData);
}

// ---------------------------------------------------------------------------
// * Insert INTO {table-name}
InsertStatement::InsertStatement(DBProcessor *aDbp)
    : SQLStatement{aDbp, Keywords::insert_kw} {}

bool InsertStatement::recognize(Tokenizer &aTokenizer) {
  TokenSequencer theSeq(aTokenizer);
  return theSeq.currentIsNoSkip({Keywords::insert_kw, Keywords::into_kw});
}

StatusResult InsertStatement::parse(Tokenizer &aTokenizer) {
  StatusResult theResult;
  TokenSequencer theSeq{aTokenizer};

  if (theSeq.currentIs({Keywords::insert_kw, Keywords::into_kw})) {
    Token &theToken = aTokenizer.current();
    if (TokenType::identifier == theToken.type) {
      // Get name of the table/entity being inserted into
      identifierData = theToken.data;
      uint32_t theHash = Helpers::hashString(identifierData);
      aTokenizer.next();
      if (aTokenizer.skipIf(left_paren)) {
        StringList theAttrList;
        if (theSeq.parseAttributeList(theAttrList)) {
          aTokenizer.skipIf(right_paren);
          if (aTokenizer.skipIf(Keywords::values_kw)) {
            if (aTokenizer.skipIf(left_paren)) {
              while (theSeq) {
                ValuesList theValues;
                theSeq.parseValuesList(theValues);
                // with whiteSpace
                // with emplace_back could cause a leak of this ptr
                rows.push_back(std::make_unique<Row>(theHash));

                if (theAttrList.size() == theValues.size()) {
                  for (size_t i{0}; i < theValues.size(); ++i) {
                    rows.back()->insert(theAttrList[i], theValues[i]);
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return theResult;
}

StatusResult InsertStatement::run(
    [[maybe_unused]] std::ostream &anOutput) const {
  return dbp->insertIntoTable(identifierData, rows);
}
// -----------------------------------------------------------
SelectStatement::SelectStatement(DBProcessor *aDbp, Keywords aType)
    : SQLStatement{aDbp, aType} {}
// StatusResult SelectStatement::parseKeyWord(Tokenizer &aTokenizer) {}

StatusResult SelectStatement::parseTableName(Tokenizer &aTokenizer,
                                             std::string &aTableName) {
  StatusResult theResult{Errors::identifierExpected};
  if (TokenType::identifier == aTokenizer.current().type) {
    std::string &theTableName = aTokenizer.current().data;
    if (dbp->entityExistsInDB(theTableName)) {
      aTableName = theTableName;
      aTokenizer.next();
      theResult.error = Errors::noError;
    } else {
      theResult.error = Errors::unknownEntity;
    }
    // *in order to use nextIs(), dont call tokenizer.next()
  }
  return theResult;
}

StatusResult SelectStatement::parseFieldList(Tokenizer &aTokenizer) {
  StatusResult theResult{Errors::identifierExpected};
  StringList theFieldList;
  ParseHelper theParseHelper{aTokenizer};
  Token &theToken = aTokenizer.current();

  if (star == theToken.data) {
    theFieldList.push_back(theToken.data);
    aTokenizer.next();
    theResult.error = Errors::noError;
  } else if (TokenType::identifier == theToken.type) {
    theResult = theParseHelper.parseIdentifierList(theFieldList);
  }
  if (theResult) {
    query.setSelectedAttrs(theFieldList);
  }
  return theResult;
}

StatusResult SelectStatement::parseWhereClause(Tokenizer &aTokenizer,
                                               const Entity &anEntity) {
  const Token &curToken = aTokenizer.current();
  if (curToken.type == TokenType::identifier ||
      curToken.keyword == Keywords::not_kw) {
    if (!query.parseExpressions(aTokenizer, anEntity)) {
      return Errors::invalidExpression;
    }
  }
  return {Errors::noError};
}

StatusResult SelectStatement::parseOrderClause(Tokenizer &aTokenizer) {
  StringList theOrderList;
  ParseHelper theParseHelper{aTokenizer};
  auto theResult = theParseHelper.parseIdentifierList(theOrderList);
  if (theResult) {
    query.setOrderBy(theOrderList);
  }
  return theResult;
}

StatusResult SelectStatement::parseLimitClause(Tokenizer &aTokenizer) {
  Token &theLimitToken = aTokenizer.current();
  if (TokenType::number == theLimitToken.type &&
      Helpers::isNumber<int>(theLimitToken.data)) {
    query.setLimit(std::stoi(theLimitToken.data));
    return {Errors::noError};
  }
  return {Errors::integerExpected};
}

// {table-nameA}.{attrNameA}
StatusResult SelectStatement::parseSubField(Tokenizer &aTokenizer,
                                            Operand &anOperand) {
  ParseHelper theParseHelper{aTokenizer};
  if (aTokenizer.current().type == TokenType::identifier) {
    std::string &theTableName =
        aTokenizer.current().data;  // validate table exists
    Entity theEntity = dbp->createEntityFromStream(theTableName);
    aTokenizer.next();
    aTokenizer.skipIf(dot);
    theParseHelper.parseOperand(theEntity, anOperand);
    return {Errors::noError};
  }
  return {Errors::syntaxError};
}

// {table-nameA}.{attrNameA} = {table-nameB}.{attrNameB}
StatusResult SelectStatement::parseEquation(Tokenizer &aTokenizer,
                                            JoinList &aJoins) {
  ParseHelper theParseHelper{aTokenizer};
  Operand theLHS;
  Operand theRHS;
  Operators theOp;
  std::vector<Logical> theLogicalOps;  // TODO: leave this empty for now

  if (parseSubField(aTokenizer, theLHS)) {
    theParseHelper.parseOperator(theOp);
    if (parseSubField(aTokenizer, theRHS)) {
      aTokenizer.skipIf(comma);
      aJoins.back().exprs.push_back(
          std::make_unique<Expression>(theLHS, theOp, theRHS, theLogicalOps));
      return {Errors::noError};
    }
  }
  return {Errors::syntaxError};
}

// * select first_name, last_name, title from Users
// * left join Books on Users.id=Books.user_id order by last_name
StatusResult SelectStatement::parseJoinClause(Tokenizer &aTokenizer) {
  StatusResult theResult{Errors::syntaxError};
  TokenSequencer theSeq{aTokenizer};
  ParseHelper theParseHelper{aTokenizer};

  std::string theJoinTableName;
  Keywords theJoinType;
  JoinList theJoins;
  while (in_array<Keywords>(gJoinTypes, aTokenizer.current().keyword)) {
    theJoinType = aTokenizer.current().keyword;
    aTokenizer.next();
    if (theSeq.currentIs({Keywords::join_kw})) {
      if ((theResult = parseTableName(aTokenizer, theJoinTableName))) {
        if (theSeq.currentIs({Keywords::on_kw})) {
          // *parse EntityA.attr = EntityB.attr
          theJoins.emplace_back(theJoinTableName, theJoinType);
          if (parseEquation(aTokenizer, theJoins)) {
            theResult.error = Errors::noError;
          }
        }
      }
    }
  }
  if (theResult) {
    query.setJoins(theJoins);
  }
  return theResult;
}

// * SELECT ... from {table-name} order by ...;
bool SelectStatement::recognize(Tokenizer &aTokenizer) {
  TokenSequencer theSeq(aTokenizer);
  return theSeq.currentIsNoSkip({Keywords::select_kw});
}

StatusResult SelectStatement::parse(Tokenizer &aTokenizer) {
  StatusResult theResult;
  TokenSequencer theSeq{aTokenizer};

  Entity theEntity;

  if (theSeq.currentIs({Keywords::select_kw})) {
    if (!parseFieldList(aTokenizer)) {
      return {Errors::identifierExpected};
    }
    // from
    if (aTokenizer.skipIf(Keywords::from_kw)) {
      std::string theTableName;
      theResult = parseTableName(aTokenizer, theTableName);
      if (!theResult) {
        return theResult;
      }
      theEntity = dbp->createEntityFromStream(theTableName);
      query.setTableName(theTableName);
      query.setEntity(theEntity);
    }
    // where, orderby, limit
    while (aTokenizer.remaining()) {
      if (theSeq.clear().currentIs({Keywords::where_kw})) {
        theResult = parseWhereClause(aTokenizer, theEntity);
      } else if (theSeq.clear().currentIs(
                     {Keywords::order_kw, Keywords::by_kw})) {
        theResult = parseOrderClause(aTokenizer);
      } else if (theSeq.clear().currentIs({Keywords::limit_kw})) {
        theResult = parseLimitClause(aTokenizer);
      } else if (in_array<Keywords>(gJoinTypes, aTokenizer.peek(0).keyword) &&
                 (Keywords::join_kw == aTokenizer.peek(1).keyword)) {
        theResult = parseJoinClause(aTokenizer);
      } else {
        aTokenizer.next();
      }
      if (!theResult) {
        return theResult;
      }
      aTokenizer.skipIf(semicolon);
    }
  } else {
    theResult.error = Errors::invalidArguments;
  }
  return theResult;
}

StatusResult SelectStatement::run(
    [[maybe_unused]] std::ostream &anOutput) const {
  if (nullptr == dbp->getActiveDB()) {
    return {Errors::noDatabaseInUse};
  }
  RowCollection theRows;
  return dbp->getSqlProcessor().showQuery(query, theRows);
}

// * UPDATE Users set zipcode="12345" WHERE id=10;
// * Update tableName, set attrName = xxx  where expression;
UpdateStatement::UpdateStatement(DBProcessor *aDbp)
    : SelectStatement{aDbp, Keywords::update_kw} {}

bool UpdateStatement::recognize(Tokenizer &aTokenizer) {
  TokenSequencer theSeq(aTokenizer);
  return theSeq.currentIsNoSkip({Keywords::update_kw});
}

StatusResult UpdateStatement::parse(Tokenizer &aTokenizer) {
  StatusResult theResult;
  TokenSequencer theSeq{aTokenizer};
  Entity theEntity;

  if (theSeq.currentIs({Keywords::update_kw})) {
    std::string theTableName;
    theResult = parseTableName(aTokenizer, theTableName);
    if (!theResult) {
      return theResult;
    }
    theEntity = dbp->createEntityFromStream(theTableName);
    query.setTableName(theTableName);
    query.setEntity(theEntity);

    if (aTokenizer.skipIf(Keywords::set_kw)) {
      if (TokenType::identifier == aTokenizer.current().type) {
        std::string theAttrName = aTokenizer.current().data;
        aTokenizer.next();
        if (auto *theAttr = theEntity.getAttribute(theAttrName)) {
          if (aTokenizer.skipIf('=')) {
            std::string theStrVal = aTokenizer.current().data;
            Value theUpdateVal =
                Helpers::toValue(theStrVal, theAttr->getType());
            query.setUpdateAttrs(theAttrName, theUpdateVal);
            aTokenizer.next();
          } else {
            theResult.error = Errors::invalidExpression;
          }
        } else {
          theResult.error = Errors::invalidAttribute;
        }
      }
    }
    if (theSeq.currentIs({Keywords::where_kw})) {
      theResult = parseWhereClause(aTokenizer, theEntity);
    }
    aTokenizer.skipIf(semicolon);

  } else {
    theResult.error = Errors::invalidArguments;
  }
  return theResult;
}

StatusResult UpdateStatement::run(
    [[maybe_unused]] std::ostream &anOutput) const {
  if (nullptr == dbp->getActiveDB()) {
    return {Errors::noDatabaseInUse};
  }
  RowCollection theRows;
  return dbp->getSqlProcessor().updateQuery(query, theRows);
}

// ----------------------------------------------------
// * DELETE FROM Users WHERE zipcode>92000;
DeleteStatement::DeleteStatement(DBProcessor *aDbp)
    : SelectStatement{aDbp, Keywords::delete_kw} {}

bool DeleteStatement::recognize(Tokenizer &aTokenizer) {
  TokenSequencer theSeq{aTokenizer};
  return theSeq.currentIsNoSkip({Keywords::delete_kw});
}

StatusResult DeleteStatement::parse(Tokenizer &aTokenizer) {
  StatusResult theResult;
  TokenSequencer theSeq{aTokenizer};
  Entity theEntity;

  if (theSeq.currentIs({Keywords::delete_kw})) {
    if (aTokenizer.skipIf(Keywords::from_kw)) {
      std::string theTableName;
      theResult = parseTableName(aTokenizer, theTableName);
      if (!theResult) {
        return theResult;
      }
      theEntity = dbp->createEntityFromStream(theTableName);
      query.setEntity(theEntity);
      query.setTableName(theTableName);
    }
    if (theSeq.currentIs({Keywords::where_kw})) {
      theResult = parseWhereClause(aTokenizer, theEntity);
    }
    aTokenizer.skipIf(semicolon);

  } else {
    theResult.error = Errors::invalidArguments;
  }
  return theResult;
}

StatusResult DeleteStatement::run(
    [[maybe_unused]] std::ostream &anOutput) const {
  if (nullptr == dbp->getActiveDB()) {
    return {Errors::noDatabaseInUse};
  }
  RowCollection theRows;
  return dbp->getSqlProcessor().deleteQuery(query, theRows);
}

// ---------------------------------------------------------------------------
// * 5. show indexes
ShowAllIdxStatement::ShowAllIdxStatement(DBProcessor *aDbp)
    : SelectStatement{aDbp, Keywords::show_kw} {}

StatusResult ShowAllIdxStatement::parse(Tokenizer &aTokenizer) {
  aTokenizer.next(2);
  return {Errors::noError};
}

bool ShowAllIdxStatement::recognize(Tokenizer &aTokenizer) {
  TokenSequencer theSeq(aTokenizer);
  return theSeq.currentIsNoSkip({Keywords::show_kw, Keywords::indexes_kw});
}

StatusResult ShowAllIdxStatement::run(
    [[maybe_unused]] std::ostream &anOutput) const {
  return dbp->showIndexes();
}

// ---------------------------------------------------------------------------
// * 6. show index {attr-name} from {table-name}
// show index id from Users;
ShowIdxStatement::ShowIdxStatement(DBProcessor *aDbp)
    : SelectStatement{aDbp, Keywords::show_kw} {}

StatusResult ShowIdxStatement::parse(Tokenizer &aTokenizer) {
  TokenSequencer theSeq{aTokenizer};
  ParseHelper theParseHelper{aTokenizer};

  StatusResult theResult{Errors::identifierExpected};
  while (aTokenizer.remaining()) {
    if (theSeq.currentIs({Keywords::show_kw, Keywords::index_kw})) {
      if (TokenType::identifier == aTokenizer.current().type) {
        theResult = theParseHelper.parseIdentifierList(fieldList);
        aTokenizer.skipIf(Keywords::from_kw);
        if (TokenType::identifier == aTokenizer.current().type) {
          theResult = parseTableName(aTokenizer, tableName);
          aTokenizer.skipIf(semicolon);
        }
      }
    }
    aTokenizer.next();
  }
  return theResult;
}

bool ShowIdxStatement::recognize(Tokenizer &aTokenizer) {
  TokenSequencer theSeq(aTokenizer);
  return theSeq.currentIsNoSkip({Keywords::show_kw, Keywords::index_kw});
}

StatusResult ShowIdxStatement::run(
    [[maybe_unused]] std::ostream &anOutput) const {
  return dbp->showIndexFromTable(tableName, fieldList);
}

}  // namespace ECE141
