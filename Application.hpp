/**
 * @file Application.hpp
 * @author Yifan Wu, Samuel Woo
 * @brief
 * @version 0.9
 * @date 2022-04-18
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef Application_hpp
#define Application_hpp

#include <memory>
#include <string>
#include <iosfwd>

#include "CmdProcessor.hpp"
#include "Config.hpp"
#include "DBProcessor.hpp"
#include "Database.hpp"
#include "FolderView.hpp"
#include "keywords.hpp"

namespace ECE141 {

class Statement;
class Tokenizer;
class StatusResult;

bool isKnown(Keywords aKeyword);

class Application : public CmdProcessor {
 public:
  explicit Application(std::ostream &anOutput);
  ~Application() override;

  // delete copy and move
  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;
  Application(Application &&) = delete;
  Application &operator=(Application &&) = delete;

  // app api...
  static std::string getVersion() { return Config::getVersion(); }

  virtual StatusResult handleInput(std::istream &anInput);
  CmdProcessor *recognizes(Tokenizer &aTokenizer) override;
  Statement *makeStatement(Tokenizer &aTokenizer,
                           StatusResult &aResult) override;
  StatusResult run(Statement *aStmt) override;

  [[nodiscard]] bool dbExists(const std::string &aDBName) const;
  [[nodiscard]] bool hasActiveDB() const;
  Database *getDatabaseInUse();
  void releaseDatabase();

  // database level operation
  [[nodiscard]] StatusResult createDatabase(const std::string &aName);
  [[nodiscard]] StatusResult dropDatabase(const std::string &aName);
  [[nodiscard]] StatusResult dumpDatabase(const std::string &aName);
  [[nodiscard]] StatusResult useDatabase(const std::string &aName);
  [[nodiscard]] StatusResult showDatabases() const;

  [[nodiscard]] StatusResult showIndexes();
  

  // handle script input
  [[nodiscard]] StatusResult runScript(const std::string &aScriptName);

 protected:
  DBProcessor dbp;
  std::unique_ptr<Database> activeDB{};
  FolderView dbView;
};

}  // namespace ECE141

#endif /* Application_hpp */
