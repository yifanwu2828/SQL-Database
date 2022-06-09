/**
 * @file SQLProcessor.cpp
 * @author Yifan Wu
 * @brief
 * @version 0.3
 * @date 2022-04-17
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SQLProcessor_hpp
#define SQLProcessor_hpp

#include <iostream>

#include "CmdProcessor.hpp"
#include "Row.hpp"

namespace ECE141 {

class Statement;
class DBProcessor;
class Database;
class Application;
class DBQuery;
class Tokenizer;
class StatusResult;

Statement* createTableStmtFactory(DBProcessor* aDbp);
Statement* dropTableStmtFactory(DBProcessor* aDbp);
Statement* showTablesStmtFactory(DBProcessor* aDbp);
Statement* describeTableStmtFactory(DBProcessor* aDbp);
Statement* insertRowStmtFactory(DBProcessor* aDbp);
Statement* selectRowStmtFactory(DBProcessor* aDbp);
Statement* updateRowStmtFactory(DBProcessor* aDbp);
Statement* deleteRowStmtFactory(DBProcessor* aDbp);

class SQLProcessor : public CmdProcessor {
 public:
  SQLProcessor(std::ostream& anOutput, DBProcessor* aDbp);
  ~SQLProcessor() override = default;

  SQLProcessor(const SQLProcessor& aCopy) = delete;
  SQLProcessor& operator=(const SQLProcessor& aCopy) = delete;
  SQLProcessor(SQLProcessor&& aSrc) = delete;
  SQLProcessor& operator=(SQLProcessor&& aSrc) = delete;

  CmdProcessor* recognizes(Tokenizer& aTokenizer) override;
  Statement* makeStatement(Tokenizer& aTokenizer,
                           StatusResult& aResult) override;

  StatusResult run(Statement* aStmt) override;

  Application* getApp();
  Database* getActiveDB();

  // new ---------------------------------------------------------------
  StatusResult showQuery(const DBQuery& aQuery, RowCollection& aCollection);
  StatusResult updateQuery(const DBQuery& aQuery, RowCollection& aCollection);
  StatusResult deleteQuery(const DBQuery& aQuery, RowCollection& aCollection);

 protected:
  DBProcessor* dbp{};
};

}  // namespace ECE141
#endif /* SQLProcessor_hpp */
