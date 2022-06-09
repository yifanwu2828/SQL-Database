/**
 * @file Errors.hpp
 * @author Yifan Wu
 * @brief 
 * @version 0.9
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef Errors_h
#define Errors_h

namespace ECE141 {

enum class Errors {
  // parse related...
  keywordExpected = 100,
  identifierExpected = 110,
  illegalIdentifier = 115,
  unknownIdentifier = 120,
  unexpectedIdentifier = 125,
  joinTypeExpected = 130,
  keyExpected = 135,
  syntaxError = 140,
  unexpectedKeyword = 145,
  unexpectedValue = 150,
  valueExpected = 155,
  operatorExpected = 160,
  punctuationExpected = 165,
  eofError = 199,
  // ? custom -------------------------------------------------
  integerExpected = 166,
  invalidExpression = 167,
  invalidOperator = 168,

  // statement related...
  statementExpected = 200,
  noDatabaseSpecified = 210,

  // table/DB related...
  tableExists = 300,
  unknownEntity = 310,
  unknownTable = 320,
  unknownDatabase = 330,
  databaseExists = 340,
  databaseCreationError = 350,
  primaryKeyRequired = 360,
  // ? custom -------------------------------------------------
  databaseDoNotExists = 370,
  databaseAlreadyInUse = 380,
  noDatabaseInUse = 390,

  // type related...
  unknownType = 400,
  unknownAttribute = 405,
  invalidAttribute = 410,
  invalidArguments = 420,
  keyValueMismatch = 430,  //# of fieldNames doesn't match values...
  nonNullableValueNotGiven = 440,

  // storage/io related...
  readError = 500,
  writeError = 510,
  seekError = 520,
  storageFull = 530,
  // ? custom -------------------------------------------------
  entityBlockNumNotFound = 550,  // custom new pa3
  entityNameExist = 551,

  noEncodePerform = 560,
  noDecodePerform = 561,
    
  fileDoesNotExist = 570,

  // index related
  indexExists = 600,
  cantCreateIndex = 605,
  unknownIndex = 610,

  // command related...
  unknownCommand = 3000,
  invalidCommand = 3010,

  // general purpose...
  userTerminated = 4998,
  notImplemented = 4999,
  noError = 5000,

  unknownError = 10000
};

class StatusResult {
 public:
  Errors error;
  uint32_t value{0};

  StatusResult(Errors anError = Errors::noError) : error(anError) {}
  operator bool() const { return Errors::noError == error; }
  auto operator==(Errors anError) const -> bool { return anError == error; }
};

}  // namespace ECE141

#endif /* errors */
