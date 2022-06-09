/**
 * @file ScriptRunner.cpp
 * @author Yifan Wu
 * @brief 
 * @version 0.9
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "ScriptRunner.hpp"

#include <array>
#include <cstddef>
#include <iomanip>
#include <map>
#include <ostream>

#include "Application.hpp"
#include "Errors.hpp"

namespace ECE141 {

std::string Color::toRed(const std::string &aStr) const {
  return RED + aStr + RESET;
}

std::string Color::toGreen(const std::string &aStr) const {
  return GREEN + aStr + RESET;
}

ScriptRunner::ScriptRunner(Application &anApp) : app{anApp} {}

void ScriptRunner::showErrors(StatusResult &aResult, std::ostream &anOutput) {
  std::map<Errors, std::string> theMessages = {
      {Errors::databaseDoNotExists, "Database Do Not Exists"},
      {Errors::databaseExists, "Database exists"},
      {Errors::entityBlockNumNotFound, "Entity BlockNum Not Found"},
      {Errors::illegalIdentifier, "Illegal identifier"},
      {Errors::integerExpected, "integerExpected"},
      {Errors::invalidCommand, "invalid Command"},
      {Errors::invalidExpression, "invalid Expression"},
      {Errors::invalidOperator, "invalid Operator"},
      {Errors::noDatabaseInUse, "No Database In Use"},
      {Errors::notImplemented, "Not Implemented Error"},
      {Errors::readError, "read Error"},
      {Errors::syntaxError, "Syntax Error"},
      {Errors::tableExists, "Table Exists"},
      {Errors::unknownCommand, "Unknown command"},
      {Errors::unknownDatabase, "Unknown database"},
      {Errors::unknownError, "Unknown error"},
      {Errors::unknownIdentifier, "Unknown identifier"},
      {Errors::unknownTable, "Unknown table"},
      {Errors::writeError, "write Error"},
  };

  std::string theMessage = "Unknown Error";
  if (theMessages.find(aResult.error) != theMessages.end()) {
    theMessage = theMessages[aResult.error];
  }
  anOutput << "Error (" << static_cast<int>(aResult.error) << ") " << theMessage
           << "\n";
}

StatusResult ScriptRunner::run(std::istream &anInput, std::ostream &anOutput) {
  StatusResult theResult{Errors::noError};
  std::string theCommand;
  while (theResult && anInput) {
    std::getline(anInput, theCommand);
    std::stringstream theStream(theCommand);
    anOutput << theCommand << "\n";
    // std::cerr << theCommand << "\n";  // uncomment for Debug
    theResult = app.handleInput(theStream);
    if (theResult == Errors::userTerminated) {
      theResult.error = Errors::noError;
      break;
    }
    if (!theResult) {
      showErrors(theResult, anOutput);
      // showErrors(theResult, std::cerr); // uncomment for debug
    }
  }

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  std::array<std::string, 2> theStatus{"FAIL", "PASS"};

#elif __APPLE__ || defined __linux__ || defined __unix__
  std::array<std::string, 2> theStatus{color.toRed("FAIL"),
                                       color.toGreen("PASS")};

#endif
  anOutput << "-------------------------------------\n";
  anOutput << "Test " << std::right << std::setfill('.') << std::setw(15) << ' '
           << theStatus.at(static_cast<size_t>(theResult)) << '\n';
  return theResult;
}

}  // namespace ECE141
