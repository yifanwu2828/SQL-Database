/**
 * @file DBStatement.cpp
 * @author Yifan Wu
 * @brief
 * @version 0.3
 * @date 2022-04-16
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "DBStatement.hpp"

#include <string>
#include <map>

#include "Application.hpp"
#include "Errors.hpp"
#include "ParseHelper.hpp"
#include "TokenSequencer.hpp"
#include "Tokenizer.hpp"

namespace ECE141 {

DBStatement::DBStatement(Application *anApp, Keywords aKeywordType)
    : Statement{aKeywordType, StatementType::DataBase_Statement}, app{anApp} {}

auto DBStatement::getStatementName() const -> std::string {
  static std::map<Keywords, std::string> DBTypeMap{
      {Keywords::create_kw, "CreateDB Statement"},
      {Keywords::drop_kw, "DropDB Statement"},
      {Keywords::show_kw, "ShowDBs Statement"},
      {Keywords::use_kw, "UseDB Statement"},
      {Keywords::dump_kw, "DumpDB Statement"}};
  if (DBTypeMap.find(kwType) != DBTypeMap.end()) {
    return DBTypeMap.at(kwType);
  }
  return "DB base Statement";
}

// ---------------------------------------------------------------------------
// * CREATE DATABASE {db-name};
CreateDBStatement::CreateDBStatement(Application *anApp)
    : DBStatement{anApp, Keywords::create_kw} {}

StatusResult CreateDBStatement::parse(Tokenizer &aTokenizer) {
  ParseHelper theParseHelper{aTokenizer};
  return theParseHelper.parseThird(identifierData);
}

bool CreateDBStatement::recognize(Tokenizer &aTokenizer) {
  TokenSequencer theSeq(aTokenizer);
  return theSeq.currentIsNoSkip({Keywords::create_kw, Keywords::database_kw});
}

StatusResult CreateDBStatement::run(
    [[maybe_unused]] std::ostream &anOutput) const {
  return app->createDatabase(identifierData);
}

// ---------------------------------------------------------------------------
// * DROP DATABASE {db-name};
DropDBStatement::DropDBStatement(Application *anApp)
    : DBStatement{anApp, Keywords::drop_kw} {}

StatusResult DropDBStatement::parse(Tokenizer &aTokenizer) {
  ParseHelper theParseHelper{aTokenizer};
  return theParseHelper.parseThird(identifierData);
}

bool DropDBStatement::recognize(Tokenizer &aTokenizer) {
  TokenSequencer theSeq(aTokenizer);
  return theSeq.currentIsNoSkip({Keywords::drop_kw, Keywords::database_kw});
}

StatusResult DropDBStatement::run(
    [[maybe_unused]] std::ostream &anOutput) const {
  return app->dropDatabase(identifierData);
}

// ---------------------------------------------------------------------------
//* DUMP
DumpDBStatement::DumpDBStatement(Application *anApp)
    : DBStatement{anApp, Keywords::dump_kw} {}

StatusResult DumpDBStatement::parse(Tokenizer &aTokenizer) {
  ParseHelper theParseHelper{aTokenizer};
  return theParseHelper.parseThird(identifierData);
}

bool DumpDBStatement::recognize(Tokenizer &aTokenizer) {
  TokenSequencer theSeq(aTokenizer);
  return theSeq.currentIsNoSkip({Keywords::dump_kw, Keywords::database_kw});
}

StatusResult DumpDBStatement::run(
    [[maybe_unused]] std::ostream &anOutput) const {
  return app->dumpDatabase(identifierData);
}

// ---------------------------------------------------------------------------
//* SHOW databases
ShowDBStatement::ShowDBStatement(Application *anApp)
    : DBStatement{anApp, Keywords::show_kw} {}

StatusResult ShowDBStatement::parse(Tokenizer &aTokenizer) {
  aTokenizer.next(2);
  return {Errors::noError};
}
bool ShowDBStatement::recognize(Tokenizer &aTokenizer) {
  TokenSequencer theSeq(aTokenizer);
  return theSeq.currentIsNoSkip({Keywords::show_kw, Keywords::databases_kw});
}

StatusResult ShowDBStatement::run(
    [[maybe_unused]] std::ostream &anOutput) const {
  return app->showDatabases();
}

// ---------------------------------------------------------------------------
// * 4. USE {db-name}
UseDBStatement::UseDBStatement(Application *anApp)
    : DBStatement{anApp, Keywords::use_kw} {}

StatusResult UseDBStatement::parse(Tokenizer &aTokenizer) {
  ParseHelper theParseHelper{aTokenizer};
  return theParseHelper.parseSecond(identifierData);
}

bool UseDBStatement::recognize(Tokenizer &aTokenizer) {
  TokenSequencer theSeq(aTokenizer);
  return theSeq.currentIsNoSkip({Keywords::use_kw});
}

StatusResult UseDBStatement::run(
    [[maybe_unused]] std::ostream &anOutput) const {
  return app->useDatabase(identifierData);
}

}  // namespace ECE141
