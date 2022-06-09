/**
 * @file Statement.cpp
 * @author Yifan Wu
 * @brief 
 * @version 0.9
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "Statement.hpp"

#include "Tokenizer.hpp"
#include "Errors.hpp"

namespace ECE141 {

Statement::Statement(Keywords aKwType, StatementType aStmtType)
    : kwType{aKwType}, stmtType{aStmtType} {}

// USE: -------------------
StatusResult Statement::parse([[maybe_unused]] Tokenizer &aTokenizer) {
  return {Errors::noError};
}

StatusResult Statement::run([[maybe_unused]] std::ostream &aStream) const {
  return {Errors::noError};
}

Keywords Statement::getKwType() const { return kwType; }
StatementType Statement::getStmtType() const { return stmtType; }
std::string Statement::getIdentifier() const { return identifierData; }
std::string Statement::getStatementName() const { return "base Statement"; }

// ---------------------------------------------------------------------------
// * RUN {script-file};
RunScriptStatement::RunScriptStatement(Keywords aKwType) : Statement{aKwType} {}

StatusResult RunScriptStatement::parse(Tokenizer &aTokenizer) {
    while(aTokenizer.currentIndex() < aTokenizer.size()) {
        identifierData += aTokenizer.current().data;
        aTokenizer.next();
    }
    return {Errors::noError};
}


}  // namespace ECE141
