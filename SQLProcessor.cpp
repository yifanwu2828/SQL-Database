/**
 * @file SQLProcessor.cpp
 * @author Yifan Wu
 * @brief
 * @version 0.9
 * @date 2022-04-17
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "SQLProcessor.hpp"

#include <functional>
#include <map>
#include <string>
#include <utility>

#include "Application.hpp"
#include "Config.hpp"
#include "DBProcessor.hpp"
#include "Database.hpp"
#include "Errors.hpp"
#include "Query.hpp"
#include "SQLStatement.hpp"
#include "Statement.hpp"
#include "TabularView.hpp"
#include "Timer.hpp"
#include "Tokenizer.hpp"
#include "keywords.hpp"

namespace ECE141 {

using TableStmtFactory = std::function<Statement*(DBProcessor*)>;
// * Table StatementFactory
Statement* createTableStmtFactory(DBProcessor* aDbp) {
  return new CreateTableStatement(aDbp);
}

Statement* dropTableStmtFactory(DBProcessor* aDbp) {
  return new DropTableStatement(aDbp);
}

Statement* showTablesStmtFactory(DBProcessor* aDbp) {
  return new ShowTableStatement(aDbp);
}

Statement* describeTableStmtFactory(DBProcessor* aDbp) {
  return new DescribeTableStatement(aDbp);
}

Statement* insertRowStmtFactory(DBProcessor* aDbp) {
  return new InsertStatement(aDbp);
}

Statement* selectRowStmtFactory(DBProcessor* aDbp) {
  return new SelectStatement(aDbp);
}

Statement* updateRowStmtFactory(DBProcessor* aDbp) {
  return new UpdateStatement(aDbp);
}

Statement* deleteRowStmtFactory(DBProcessor* aDbp) {
  return new DeleteStatement(aDbp);
}

Statement* showAllIdxStmtFactory(DBProcessor* aDbp) {
  return new ShowAllIdxStatement(aDbp);
}

Statement* showIdxStmtFactory(DBProcessor* aDbp) {
  return new ShowIdxStatement(aDbp);
}

// ---------------------------------------------------------------
// SQLProcessor class
SQLProcessor::SQLProcessor(std::ostream& anOutput, DBProcessor* aDbp)
    : CmdProcessor(anOutput, "SQL Processor"), dbp{aDbp} {}

// helper
Application* SQLProcessor ::getApp() { return dbp->getApp(); }
Database* SQLProcessor ::getActiveDB() {
  return dbp->getApp()->getDatabaseInUse();
}

// virtual --------------------------------------
CmdProcessor* SQLProcessor::recognizes(Tokenizer& aTokenizer) {
  if (CreateTableStatement::recognize(aTokenizer) ||
      ShowTableStatement::recognize(aTokenizer) ||
      DropTableStatement::recognize(aTokenizer) ||
      DescribeTableStatement::recognize(aTokenizer) ||
      InsertStatement::recognize(aTokenizer) ||
      SelectStatement::recognize(aTokenizer) ||
      UpdateStatement::recognize(aTokenizer) ||
      DeleteStatement::recognize(aTokenizer) ||
      ShowAllIdxStatement::recognize(aTokenizer) ||
      ShowIdxStatement::recognize(aTokenizer)) {
    return this;  // sqlProcessor
  }
  return nullptr;  // can't recognize token
}

StatusResult SQLProcessor::run(Statement* aStmt) {
  return aStmt->run(output);  // from cmdProcessor
}

using KeywordPair = std::pair<Keywords, Keywords>;
Statement* SQLProcessor::makeStatement(Tokenizer& aTokenizer,
                                       [[maybe_unused]] StatusResult& aResult) {
  static std::map<Keywords, TableStmtFactory> factories{
      {Keywords::create_kw, createTableStmtFactory},
      {Keywords::delete_kw, deleteRowStmtFactory},
      {Keywords::describe_kw, describeTableStmtFactory},
      {Keywords::drop_kw, dropTableStmtFactory},
      {Keywords::insert_kw, insertRowStmtFactory},
      {Keywords::select_kw, selectRowStmtFactory},
      {Keywords::update_kw, updateRowStmtFactory},
  };

  static std::map<KeywordPair, TableStmtFactory> showFactories{
      {{Keywords::show_kw, Keywords::index_kw}, showIdxStmtFactory},
      {{Keywords::show_kw, Keywords::indexes_kw}, showAllIdxStmtFactory},
      {{Keywords::show_kw, Keywords::tables_kw}, showTablesStmtFactory},
  };

  const Keywords& theFirstKW = aTokenizer.current().keyword;
  if (factories.find(theFirstKW) != factories.end()) {
    if (Statement* theStatement = (factories.at(theFirstKW))(dbp)) {
      if (theStatement->parse(aTokenizer)) {
        return theStatement;
      }
    }
  } else {
    const Keywords& theSecondKW = aTokenizer.peek(1).keyword;
    KeywordPair theKVPair = std::make_pair(theFirstKW, theSecondKW);
    if (showFactories.find(theKVPair) != showFactories.end()) {
      if (Statement* theStatement = showFactories.at(theKVPair)(dbp)) {
        if (theStatement->parse(aTokenizer)) {
          return theStatement;
        }
      }
    }
  }
  return nullptr;
}

// new ----------------------------------------------------------
// at the beginning the rowCollections is just empty
StatusResult SQLProcessor::showQuery(const DBQuery& aQuery,
                                     RowCollection& aCollection) {
  StatusResult theResult{Errors::noDatabaseInUse};
  if (auto* theActiveDB = getActiveDB()) {
    theResult = theActiveDB->selectRow(aQuery, aCollection);
    TabularView theTableView(output, aCollection, aQuery.getFieldList());
    theTableView.show();
  }
  return theResult;
}

StatusResult SQLProcessor::updateQuery(const DBQuery& aQuery,
                                       RowCollection& aCollection) {
  StatusResult theResult{Errors::noDatabaseInUse};
  if (auto* theActiveDB = getActiveDB()) {
    theResult = theActiveDB->updateRow(aQuery, aCollection);
    TabularView theTableView(output, aCollection, aQuery.getFieldList());
    theTableView.showFooter(aCollection.size(), Config::getTimer().elapsed());
  }
  return theResult;
}

StatusResult SQLProcessor::deleteQuery(const DBQuery& aQuery,
                                       RowCollection& aCollection) {
  StatusResult theResult{Errors::noDatabaseInUse};
  if (auto* theActiveDB = getActiveDB()) {
    theResult = theActiveDB->deleteRow(aQuery, aCollection);
    TabularView theTableView(output, aCollection, aQuery.getFieldList());
    theTableView.showFooter(aCollection.size(), Config::getTimer().elapsed());
  }
  return theResult;
}

}  // namespace ECE141
