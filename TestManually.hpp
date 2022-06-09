/**
 * @file TestManually.hpp
 * @author Yifan Wu
 * @brief 
 * @version 0.9
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef TestManually_h
#define TestManually_h

#include <map>
#include <sstream>

#include "Application.hpp"
#include "Errors.hpp"

void showError(ECE141::StatusResult &aResult, std::ostream &anOutput) {
  static std::map<ECE141::Errors, std::string> theErrorMessages = {
      {ECE141::Errors::illegalIdentifier, "Illegal identifier"},
      {ECE141::Errors::unknownIdentifier, "Unknown identifier"},
      {ECE141::Errors::databaseExists, "Database exists"},
      {ECE141::Errors::tableExists, "Table Exists"},
      {ECE141::Errors::syntaxError, "Syntax Error"},
      {ECE141::Errors::unknownCommand, "Unknown command"},
      {ECE141::Errors::unknownDatabase, "Unknown database"},
      {ECE141::Errors::unknownTable, "Unknown table"},
      {ECE141::Errors::notImplemented, "Not Implemented Error"},
      {ECE141::Errors::unknownError, "Unknown error"},
      // ? custom ---------------------------------------------------------
      {ECE141::Errors::readError, "read Error"},
      {ECE141::Errors::writeError, "write Error"},
      {ECE141::Errors::noDatabaseInUse, "no Database In Use"},
      {ECE141::Errors::databaseDoNotExists, "database Do Not Exists"},
      {ECE141::Errors::entityBlockNumNotFound, "entity BlockNum Not Found"},
      {ECE141::Errors::integerExpected, "integer Expected"},
      {ECE141::Errors::invalidExpression, "invalid Expression"},
      {ECE141::Errors::fileDoesNotExist, "fileDoesNotExist"},
      
      
      };

  std::string theMessage = "Unknown Error";
  if (theErrorMessages.count(aResult.error)) {
    theMessage = theErrorMessages[aResult.error];
  }
  anOutput << "Error (" << static_cast<int>(aResult.error) << ") " << theMessage << "\n";
}

bool doManualTesting() {
  ECE141::Application theApp(std::cout);
  ECE141::StatusResult theResult{};

  std::string theUserInput;
  bool running = true;
  do {
    std::cout << "\n> ";
    if (std::getline(std::cin, theUserInput)) {
      if (theUserInput.length()) {
        std::stringstream theStream(theUserInput);
        theResult = theApp.handleInput(theStream);
        if (theResult == ECE141::Errors::userTerminated) {
          running = false;
        } else if (!theResult) {
          showError(theResult, std::cerr);
        }
      }
    }
  } while (running);
  return theResult;
}

#endif /* TestManually_h */
