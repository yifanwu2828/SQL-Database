/**
 * @file Application.cpp
 * @author Yifan Wu
 * @brief
 * @version 0.9
 * @date 2022-05-31
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "Application.hpp"

#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>

#include "Config.hpp"
#include "Errors.hpp"
#include "FolderReader.hpp"
#include "ScriptRunner.hpp"
#include "Statement.hpp"
#include "TableFormatter.hpp"
#include "Timer.hpp"
#include "Tokenizer.hpp"

namespace fs = std::filesystem;
namespace ECE141 {

std::array<size_t, 3> Config::cacheSize = {0, 0, 0};

Application::Application(std::ostream &anOutput)
    : CmdProcessor(anOutput, "App"),
      dbp(anOutput, this),
      dbView{Config::getStoragePath(), Config::getDBExtension()} {}

Application::~Application() { releaseDatabase(); }

[[nodiscard]] bool Application::hasActiveDB() const {
  return activeDB != nullptr;
}
Database *Application::getDatabaseInUse() { return activeDB.get(); }
void Application::releaseDatabase() { activeDB.reset(nullptr); }

// USE: -----------------------------------------------------

bool isKnown(Keywords aKeyword) {
  static std::set<Keywords> theKnown{
      Keywords::help_kw,
      Keywords::quit_kw,
      Keywords::run_kw,
      Keywords::version_kw,
  };
  return theKnown.find(aKeyword) != theKnown.end();
}

auto Application::recognizes(Tokenizer &aTokenizer) -> CmdProcessor * {
  if (isKnown(aTokenizer.current().keyword)) {
    return this;  // application itself
  }
  // will return dbProcessor -> sqlProcessor -> nullptr
  return dbp.recognizes(aTokenizer);
}

auto Application::run(Statement *aStatement) -> StatusResult {
  StatusResult theResult{Errors::noError};
  switch (aStatement->getKwType()) {
    case Keywords::help_kw:
      output << "Help system available\n";
      break;
    case Keywords::quit_kw:
      output << "DB::141 is shutting down\n";
      return {Errors::userTerminated};
      break;
    case Keywords::run_kw:
      theResult = runScript(aStatement->getIdentifier());
      break;
    case Keywords::version_kw:
      output << "Version " << getVersion() << '\n';
      break;
    default:
      break;
  }
  return theResult;
}

// USE: retrieve a statement based on given text input...
Statement *Application::makeStatement(Tokenizer &aTokenizer,
                                      [[maybe_unused]] StatusResult &aResult) {
  Token &theToken = aTokenizer.current();
  if (isKnown(theToken.keyword)) {
    aTokenizer.next();  // skip ahead...
    if (Keywords::run_kw == theToken.keyword) {
      Statement *theStatement = new RunScriptStatement(theToken.keyword);
      if (theStatement->parse(aTokenizer)) {
        return theStatement;
      }
    }
    return new Statement(theToken.keyword);
  }
  return nullptr;
}

// build a tokenizer, tokenize input, ask processors to handle...
auto Application::handleInput(std::istream &anInput) -> StatusResult {
  Tokenizer theTokenizer(anInput);
  StatusResult theResult = theTokenizer.tokenize();

  while (theResult && theTokenizer.more()) {
    Config::getTimer().reset();  // don't reset elsewhere
    // polymorphism
    if (auto *theProc = recognizes(theTokenizer)) {
      if (auto *theCmd = theProc->makeStatement(theTokenizer, theResult)) {
        theResult = theProc->run(theCmd);
        if (theResult) {
          theTokenizer.skipIf(';');
        }
        delete theCmd;
      } else {
        // recognize the cmd, but it is not valid
        return {Errors::invalidCommand};
      }
    } else {
      // can't recognize the cmd
      theResult.error = Errors::unknownCommand;
    }
  }
  return theResult;
}

bool Application::dbExists(const std::string &aDBName) const {
  return dbView.reader.exists(Config::getDBFilename(aDBName));
}

auto Application::createDatabase(const std::string &aName) -> StatusResult {
  StatusResult theResult;
  if (dbExists(aName)) {
    theResult.error = Errors::databaseExists;
    return theResult;
  }
  // create database after checking exist
  Database theDatabase = Database(aName, CreateDB{});
  theDatabase.setDebugInfo("Create Database");
  TableFormatter::printStatusRowDuration(output, theResult, 1,
                                         Config::getTimer().elapsed());
  return theResult;
}

StatusResult Application::showDatabases() const {
  // make a view, load with DB names from storage folder...
  output.setf(std::ios::left, std::ios::adjustfield);
  TableFormatter::printBreak(output, {23});
  output.width(23);
  output << "| Databases"
         << "|\n";
  dbView.show(output);
  TableFormatter::printDuration(output, Config::getTimer().elapsed());
  return StatusResult{Errors::noError};
}

// USE: call this to perform the dropping of a database (remove the file)...
auto Application::dropDatabase(const std::string &aName) -> StatusResult {
  StatusResult theResult{Errors::databaseDoNotExists};
  if (dbExists(aName)) {
    if (activeDB != nullptr) {
      activeDB->setDebugInfo("drop Database");
      releaseDatabase();
    }
    if (!dbView.reader.remove(Config::getDBFilename(aName))) {
      std::cerr << "remove fail!!";
    }
    theResult.error = Errors::noError;
  }
  TableFormatter::printStatusRowDuration(output, theResult, 0,
                                         Config::getTimer().elapsed());
  return theResult;
}

// USE: DB dump all storage blocks
auto Application::dumpDatabase(const std::string &aName) -> StatusResult {
  StatusResult theResult{Errors::databaseDoNotExists};
  if (dbExists(aName)) {
    if (hasActiveDB()) {
      std::string theActiveDBName = activeDB->getName();
      activeDB = std::make_unique<Database>(aName, OpenDB{});
      theResult = activeDB->dump(output);
      activeDB = std::make_unique<Database>(theActiveDBName, OpenDB{});
    } else {
      Database theDatabase = Database(aName, OpenDB{});
      theDatabase.setDebugInfo("Dump Database");
      theResult = theDatabase.dump(output);
    }
    TableFormatter::printDuration(output, Config::getTimer().elapsed());
  }
  return theResult;
}

// USE: call DB object to be loaded into memory...
auto Application::useDatabase(const std::string &aName) -> StatusResult {
  StatusResult theResult{Errors::databaseDoNotExists};
  if (dbExists(aName)) {
    // db in use
    if (hasActiveDB()) {
      // use same database -> no change
      if (activeDB->getName() == aName) {
        TableFormatter::printDBChange(output, false);
      } else {
        // invoke ~Database() to save data
        // that switching between different databases;
        activeDB = std::make_unique<Database>(aName, OpenDB{});
        TableFormatter::printDBChange(output, true);
      }
    } else {
      activeDB = std::make_unique<Database>(aName, OpenDB{});
      TableFormatter::printDBChange(output, true);
    }
    activeDB->setDebugInfo("use Database");
    theResult.error = Errors::noError;
  }
  return theResult;
}

auto Application::runScript(const std::string &aScriptName) -> StatusResult {
  StatusResult theResult{Errors::fileDoesNotExist};

  auto theFilePath = fs::path(aScriptName);
  if (!theFilePath.has_parent_path()) {
    theFilePath = fs::absolute("test" / theFilePath);
    std::cerr << theFilePath << '\n';
  }

  if (".txt" != theFilePath.extension()) {
    std::cerr << "test script should be a .txt file!\n";
    return theResult;
  }

  std::fstream theScript(theFilePath.c_str());  // Works with files in abs path
  ScriptRunner theRunner(*this);
  return theRunner.run(theScript, std::cerr);
}

}  // namespace ECE141
