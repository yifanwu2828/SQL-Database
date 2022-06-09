/**
 * @file Filters.cpp
 * @author Yifan Wu, Samuel Woo
 * @brief
 * @version 0.7
 * @date 2022-05-18
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "Filters.hpp"

#include <algorithm>
#include <deque>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "Compare.hpp"
#include "Errors.hpp"
#include "Helpers.hpp"
#include "ParseHelper.hpp"

namespace ECE141 {

using Comparator = bool (*)(Value &aLHS, Value &aRHS);

bool equals(Value &aLHS, Value &aRHS) {
  bool theResult = false;

  std::visit(
      [&](auto const &aLeft) {
        std::visit(
            [&](auto const &aRight) { theResult = isEqual(aLeft, aRight); },
            aRHS);
      },
      aLHS);
  return theResult;
}

bool notEquals(Value &aLHS, Value &aRHS) { return !equals(aLHS, aRHS); }

bool lessThan(Value &aLHS, Value &aRHS) {
  bool theResult = false;

  std::visit(
      [&](auto const &aLeft) {
        std::visit(
            [&](auto const &aRight) { theResult = isLess(aLeft, aRight); },
            aRHS);
      },
      aLHS);
  return theResult;
}

bool lessEquals(Value &aLHS, Value &aRHS) {
  return lessThan(aLHS, aRHS) || equals(aLHS, aRHS);
}

bool greaterThan(Value &aLHS, Value &aRHS) { return !lessEquals(aLHS, aRHS); }

bool greaterEquals(Value &aLHS, Value &aRHS) { return !lessThan(aLHS, aRHS); }

static std::map<Operators, Comparator> comparators{
    {Operators::equal_op, equals},   {Operators::notequal_op, notEquals},
    {Operators::lt_op, lessThan},    {Operators::lte_op, lessEquals},
    {Operators::gt_op, greaterThan}, {Operators::gte_op, greaterEquals},
};

// -----------------------------------------------------------------
Operand::Operand(std::string aName, TokenType aType, Value &aValue, size_t anId)
    : ttype(aType),
      dtype(DataTypes::varchar_type),
      name(std::move(aName)),
      value(aValue),
      entityId(anId) {}

// -----------------------------------------------------------------
// Expression interface
Expression::Expression(Operand &aLHSOperand, Operators &anOp,
                       Operand &aRHSOperand, std::vector<Logical> &aLogOps)
    : lhs{aLHSOperand}, rhs{aRHSOperand}, op{anOp}, logics{aLogOps} {}
bool Expression::operator()(const KeyValues &aMap) const {
  Value theLHS{lhs.value};
  Value theRHS{rhs.value};
  Operators theOp{op};

  if (TokenType::identifier == lhs.ttype) {
    if (aMap.find(lhs.name) != aMap.end()) {
      // std::cout << "lhs.name found: " << lhs.name << "\n";
      theLHS = aMap.at(lhs.name);  // get row value
    } else {
      std::cerr << "lhs.name NOT FOUND:" << lhs.name << "\n";
    }
  }

  if (TokenType::identifier == rhs.ttype) {
    if (aMap.find(rhs.name) != aMap.end()) {
      // std::cerr << "rhs.name found: " << rhs.name << "\n";
      theRHS = aMap.at(rhs.name);  // get row value
    } else {
      std::cerr << "rhs.name NOT FOUND:" << rhs.name << "\n";
    }
  }

  // If there are an odd number of NOTS, switch the operator to its inverse
  if ((std::count(logics.begin(), logics.end(), Logical::not_op) % 2) != 0) {
    theOp = Helpers::oppositeOpOf(theOp);
  }

  return (comparators.find(theOp) != comparators.end())
             ? comparators[theOp](theLHS, theRHS)
             : false;
}

bool Expression::operator()(const KeyValues &aLHSMap,
                            const KeyValues &aRHSMap) const {
  Value theLHS{lhs.value};
  Value theRHS{rhs.value};
  Operators theOp{op};

  if (TokenType::identifier == lhs.ttype) {
    if (aLHSMap.find(lhs.name) != aLHSMap.end()) {
      // std::cout << "lhs.name found: " << lhs.name << "\n";
      theLHS = aLHSMap.at(lhs.name);  // get row value
    } else {
      std::cerr << "lhs.name NOT FOUND:" << lhs.name << "\n";
    }
  }

  if (TokenType::identifier == rhs.ttype) {
    if (aRHSMap.find(rhs.name) != aRHSMap.end()) {
      // std::cout << "rhs.name found: " << rhs.name << "\n";
      theRHS = aRHSMap.at(rhs.name);  // get row value
    } else {
      std::cerr << "rhs.name NOT FOUND:" << rhs.name << "\n";
    }
  }

  // If there are an odd number of NOTS, switch the operator to its inverse
  if ((std::count(logics.begin(), logics.end(), Logical::not_op) % 2) != 0) {
    theOp = Helpers::oppositeOpOf(theOp);
  }

  return (comparators.find(theOp) != comparators.end())
             ? comparators[theOp](theLHS, theRHS)
             : false;
}

std::ostream &operator<<(std::ostream &aStream,
                         const Expression &anExpression) {
  std::string theRhsOutput = anExpression.rhs.name;
  if (theRhsOutput.empty()) {
    theRhsOutput = Helpers::valToString(anExpression.rhs.value);
    theRhsOutput.pop_back();
  }
  std::string theLogOpStr;
  for (const auto &logOp : anExpression.logics) {
    if (kwToLogStrMap.find(logOp) != kwToLogStrMap.end()) {
      theLogOpStr += kwToLogStrMap.at(logOp);
      theLogOpStr += " ";
    } else {
      std::cerr << "logOp Not FOUND!\n";
      break;
    }
  }
  if (!theLogOpStr.empty()) {
    aStream << theLogOpStr;
  }
  aStream << anExpression.lhs.name << " "
          << ExpressionOpsToStr.at(anExpression.op) << " " << theRhsOutput
          << "\n";
  return aStream;
}

//--------------------------------------------------------------
// Filters
size_t Filters::getCount() const { return expressions.size(); }
size_t Filters::getExpressionsNum() const { return expressions.size(); }
const Expressions &Filters::getExpressions() { return expressions; }

Filters &Filters::add(Expression *anExpression) {
  // std::cout << *anExpression;  // *uncomment for debug
  expressions.push_back(std::unique_ptr<Expression>(anExpression));
  return *this;
}

bool Filters::matches(const KeyValues &aMap) const {
  return matches(expressions, aMap);
}

// compare expressions to row; return true if matches
bool Filters::matches(const Expressions &anExpressions, const KeyValues &aMap) {
  std::deque<bool> theEvalResults;
  std::deque<Logical> theAndOrOps;

  for (const auto &theExpr : anExpressions) {
    if (1 == std::count(theExpr->logics.begin(), theExpr->logics.end(),
                        Logical::and_op)) {
      theAndOrOps.emplace_back(Logical::and_op);
    } else if (1 == std::count(theExpr->logics.begin(), theExpr->logics.end(),
                               Logical::or_op)) {
      theAndOrOps.emplace_back(Logical::or_op);
    }
    theEvalResults.push_back((*theExpr)(aMap));

    //    if(Logical::and_op == theExpr->logic) {
    //        theRowResult = theRowResult && theAttrResult;
    //    }
    //    else if (Logical::or_op == theExpr->logic) {
    //        theRowResult = theRowResult || theAttrResult;
    //    }
    //    else {
    //        theRowResult = theAttrResult;
    //    }
  }
  for (size_t i{0}; i < theAndOrOps.size(); i++) {
    if (Logical::and_op == theAndOrOps[i]) {
      bool theResult = theEvalResults[i] && theEvalResults[i + 1];
      theEvalResults[i + 1] = theResult;
      theEvalResults.erase(theEvalResults.begin() + i);
      theAndOrOps.erase(theAndOrOps.begin() + i);
      i--;  // so for loop doesn't iterate out of range
    }
  }
  for (size_t i{0}; i < theAndOrOps.size(); i++) {
    if (Logical::or_op == theAndOrOps[i]) {
      bool theResult = theEvalResults[i] || theEvalResults[i + 1];
      theEvalResults[i + 1] = theResult;
      theEvalResults.erase(theEvalResults.begin() + i);
      theAndOrOps.erase(theAndOrOps.begin() + i);
      i--;  // so for loop doesn't iterate out of range
    }
  }
  if (theAndOrOps.empty() && 1 == theEvalResults.size()) {
    return theEvalResults.front();
  }
  std::cerr << "Error with sizes" << '\n';
  return false;
}

bool Filters::matches(const Expressions &anExpressions,
                      const KeyValues &aLHSMap, const KeyValues &aRHSMap) {
  std::deque<bool> theEvalResults;
  std::deque<Logical> theAndOrOps;

  for (const auto &theExpr : anExpressions) {
    if (1 == std::count(theExpr->logics.begin(), theExpr->logics.end(),
                        Logical::and_op)) {
      theAndOrOps.emplace_back(Logical::and_op);
    } else if (1 == std::count(theExpr->logics.begin(), theExpr->logics.end(),
                               Logical::or_op)) {
      theAndOrOps.emplace_back(Logical::or_op);
    }
    theEvalResults.push_back((*theExpr)(aLHSMap, aRHSMap));

    //    if(Logical::and_op == theExpr->logic) {
    //        theRowResult = theRowResult && theAttrResult;
    //    }
    //    else if (Logical::or_op == theExpr->logic) {
    //        theRowResult = theRowResult || theAttrResult;
    //    }
    //    else {
    //        theRowResult = theAttrResult;
    //    }
  }
  for (size_t i{0}; i < theAndOrOps.size(); i++) {
    if (Logical::and_op == theAndOrOps[i]) {
      bool theResult = theEvalResults[i] && theEvalResults[i + 1];
      theEvalResults[i + 1] = theResult;
      theEvalResults.erase(theEvalResults.begin() + i);
      theAndOrOps.erase(theAndOrOps.begin() + i);
      i--;  // so for loop doesn't iterate out of range
    }
  }
  for (size_t i{0}; i < theAndOrOps.size(); i++) {
    if (Logical::or_op == theAndOrOps[i]) {
      bool theResult = theEvalResults[i] || theEvalResults[i + 1];
      theEvalResults[i + 1] = theResult;
      theEvalResults.erase(theEvalResults.begin() + i);
      theAndOrOps.erase(theAndOrOps.begin() + i);
      i--;  // so for loop doesn't iterate out of range
    }
  }
  if (theAndOrOps.empty() && 1 == theEvalResults.size()) {
    return theEvalResults.front();
  }
  std::cerr << "Error with sizes" << '\n';
  return false;
}

// TODO: Add validation here...
bool validateOperands(const Operand &aLHS, const Operand &aRHS,
                      [[maybe_unused]] const Entity &anEntity) {
  if (TokenType::identifier == aLHS.ttype) {  // most common case...
    // STUDENT: Add code for validation as necessary
    return true;
  } else if (TokenType::identifier == aRHS.ttype) {
    // STUDENT: Add code for validation as necessary
    return true;
  }
  return false;
}

// STUDENT: This starting point code may need adaptation...
auto Filters::parse(Tokenizer &aTokenizer, const Entity &anEntity)
    -> StatusResult {
  StatusResult theResult{Errors::noError};
  ParseHelper theParseHelper{aTokenizer};

  while (theResult && (2 < aTokenizer.remaining())) {
    Operand theLHS;
    Operand theRHS;
    Logical theLogOp;
    std::vector<Logical> theLogicalOps;

    // Handle Logical Operators
    while (TokenType::keyword == aTokenizer.current().type) {
      theLogOp = Helpers::kwToLogOp(aTokenizer.current().keyword);
      if (Logical::no_op != theLogOp) {
        theLogicalOps.push_back(theLogOp);
        aTokenizer.next();
      } else {
        return theResult;
      }
    }

    if ((theResult = theParseHelper.parseOperand(anEntity, theLHS))) {
      Operators theOp1;
      // parse operates(>, =, < <=, >=, !=, else invalid)
      if ((theResult = theParseHelper.parseAndMergeOperator(theOp1))) {
        // parse RHS
        if ((theResult = theParseHelper.parseOperand(anEntity, theRHS))) {
          if (validateOperands(theLHS, theRHS, anEntity)) {
            add(new Expression(theLHS, theOp1, theRHS, theLogicalOps));
            if (aTokenizer.skipIf(';')) {
              break;
            }
          } else {
            theResult.error = Errors::syntaxError;
          }
        }
      }
    } else {
      theResult.error = Errors::syntaxError;
    }
  }
  return theResult;
}

}  // namespace ECE141
