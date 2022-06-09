/**
 * @file DBProcessor.hpp
 * @author Yifan Wu, Samuel Woo
 * @brief
 * @version 0.9
 * @date 2022-04-17
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef DBProcessor_hpp
#define DBProcessor_hpp

#include <iosfwd>
#include <string>

#include "BasicTypes.hpp"
#include "CmdProcessor.hpp"
#include "Row.hpp"
#include "SQLProcessor.hpp"

namespace ECE141 {

class Application;
class Database;
class Tokenizer;
class Entity;
class StatusResult;
class Statement;

Statement *createDBStmtFactory(Application *anApp);

Statement *dropDBStmtFactory(Application *anApp);

Statement *useDBsStmtFactory(Application *anApp);

Statement *showDBStmtFactory(Application *anApp);

Statement *dumpDBStmtFactory(Application *anApp);

class DBProcessor : public CmdProcessor {
 public:
  DBProcessor(std::ostream &anOutput, Application *anApp);
  ~DBProcessor() override = default;

  DBProcessor(const DBProcessor &aCopy) = delete;
  DBProcessor &operator=(const DBProcessor &aCopy) = delete;
  DBProcessor(DBProcessor &&aSrc) = delete;
  DBProcessor &operator=(DBProcessor &&aSrc) = delete;

  // cmd processor interface...
  CmdProcessor *recognizes(Tokenizer &aTokenizer) override;
  Statement *makeStatement(Tokenizer &aTokenizer,
                           StatusResult &aResult) override;
  StatusResult run(Statement *aStmt) override;

  // sqlProcessor interface...
  StatusResult createTable(Entity *anEntity);
  StatusResult describeTable(const std::string &aName);
  StatusResult dropTable(const std::string &aName);
  StatusResult showTables();
  StatusResult showIndexes();
  StatusResult showIndexFromTable(const std::string &aTableName,
                                  const StringList &aFieldList);
  StatusResult insertIntoTable(const std::string &aName,
                               const RowCollection &aRowCollect);

  // helper
  Application *getApp();
  SQLProcessor &getSqlProcessor();
  [[nodiscard]] Database *getActiveDB() const;
  [[nodiscard]] bool entityExistsInDB(const std::string &aName) const;
  [[nodiscard]] Entity createEntityFromStream(const std::string &aName) const;

 protected:
  Application *app{};
  SQLProcessor sql;
};

}  // namespace ECE141
#endif /* DBProcessor_hpp */