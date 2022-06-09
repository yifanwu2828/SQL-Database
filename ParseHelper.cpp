/**
 * @file ParseHelper.cpp
 * @author Yifan Wu
 * @brief 
 * @version 0.1
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "ParseHelper.hpp"

#include <array>
#include <cstdlib>

#include "Attribute.hpp"
#include "Entity.hpp"
#include "Errors.hpp"
#include "Filters.hpp"
#include "Helpers.hpp"
#include "Scanner.hpp"
#include "Tokenizer.hpp"

namespace ECE141 {
ParseHelper::ParseHelper(Tokenizer &aTokenizer) : tokenizer(aTokenizer) {}

auto ParseHelper::parseSecond(std::string &aStr) -> StatusResult {
  if (tokenizer.remaining() < 2) {
    return {Errors::identifierExpected};
  }
  aStr = tokenizer.tokenAt(1).data;
  tokenizer.next(2);  // skip ahead...
  return {Errors::noError};
}

auto ParseHelper::parseThird(std::string &aStr) -> StatusResult {
  if (tokenizer.remaining() < 3) {
    return {Errors::identifierExpected};
  }
  aStr = tokenizer.tokenAt(2).data;
  tokenizer.next(3);  // skip ahead...
  return {Errors::noError};
}

// USE: gets properties following the type in an attribute decl...
auto ParseHelper::parseAttributeOptions(Attribute &anAttribute)
    -> StatusResult {
  bool options = true;
  StatusResult theResult{Errors::noError};
  std::array<char, 4> thePunct{");,"};

  while (theResult && options && tokenizer.more()) {
    Token &theToken = tokenizer.current();
    switch (theToken.type) {
      case TokenType::keyword:
        switch (theToken.keyword) {
          case Keywords::auto_increment_kw:
            anAttribute.setAutoIncrement(true);
            break;
          case Keywords::primary_kw:
            anAttribute.setPrimaryKey(true);
            break;
          case Keywords::not_kw:
            tokenizer.next();
            theToken = tokenizer.current();
            if (Keywords::null_kw == theToken.keyword) {
              anAttribute.setNullable(false);
            } else
              theResult.error = Errors::syntaxError;
            break;

          default:
            break;
        }
        break;

      case TokenType::punctuation:  // fall thru...
        options = !in_array<char>(thePunct, theToken.data[0]);
        break;

      default:
        options = false;
        theResult.error = Errors::syntaxError;
    }                  // switch
    tokenizer.next();  // skip ahead...
  }                    // while
  return theResult;
}

// USE : parse an individual attribute (name type [options])
auto ParseHelper::parseAttribute(Attribute &anAttribute) -> StatusResult {
  StatusResult theResult{Errors::noError};

  if (tokenizer.more()) {
    Token &theToken = tokenizer.current();
    if (Helpers::isDatatype(theToken.keyword)) {
      DataTypes theType = Helpers::getTypeForKeyword(theToken.keyword);
      anAttribute.setDataType(theType);
      tokenizer.next();

      if (DataTypes::varchar_type == theType) {
        if ((tokenizer.skipIf(left_paren))) {
          theToken = tokenizer.current();
          tokenizer.next();
          if ((tokenizer.skipIf(right_paren))) {
            anAttribute.setSize(atoi(theToken.data.c_str()));
            // return theResult;
          }
        }
      }

      if (theResult) {
        theResult = parseAttributeOptions(anAttribute);
        if (!anAttribute.isValid()) {
          theResult.error = Errors::invalidAttribute;
        }
      }

    }  // if
    else
      theResult.error = Errors::invalidAttribute;
  }  // if
  return theResult;
}

// USE: parse a comma-sep list of (unvalidated) identifiers;
//      AUTO stop if keyword (or term)
auto ParseHelper::parseIdentifierList(StringList &aList) -> StatusResult {
  StatusResult theResult{Errors::noError};

  while (theResult && tokenizer.more()) {
    Token &theToken = tokenizer.current();
    if (TokenType::identifier == tokenizer.current().type) {
      aList.push_back(theToken.data);
      tokenizer.next();  // skip identifier...
      tokenizer.skipIf(comma);
    } else if (TokenType::keyword == theToken.type) {
      break;  // Auto stop if we see a keyword...
    } else if (tokenizer.skipIf(right_paren)) {
      break;
    } else if (semicolon == theToken.data[0]) {
      break;
    } else {
      theResult.error = Errors::syntaxError;
    }
  }
  return theResult;
}

//** USE: get a list of values (identifiers, strings, numbers...)
auto ParseHelper::parseValueList(StringList &aList) -> StatusResult {
  StatusResult theResult{Errors::noError};

  while (theResult && tokenizer.more()) {
    Token &theToken = tokenizer.current();
    if (TokenType::identifier == theToken.type ||
        TokenType::number == theToken.type) {
      aList.push_back(theToken.data);
      tokenizer.next();  // skip identifier...
      tokenizer.skipIf(comma);
    } else if (tokenizer.skipIf(right_paren)) {
      break;
    } else {
      theResult.error = Errors::syntaxError;
    }
  }
  return theResult;
}

// ---------------------------------------------------------------------
auto ParseHelper::parseOperator(Operators &anOp) -> StatusResult {
  const Token &theOpToken = tokenizer.current();
  if (theOpToken.type == TokenType::operators) {
    anOp = Helpers::toOperator(theOpToken.data);
    tokenizer.next();
    if (Operators::unknown_op == anOp) {
      return {Errors::invalidOperator};
    }
  }
  return {Errors::noError};
}

// handle <=, >=, =, !=, else invalid
auto ParseHelper::parseAndMergeOperator(Operators &anOp) -> StatusResult {
  StatusResult theResult{Errors::syntaxError};
  const Token &theTokenOp1 = tokenizer.current();
  if ((theResult = parseOperator(anOp))) {
    const Token &theTokenOp2 = tokenizer.current();
    if (theTokenOp2.type == TokenType::operators) {
      Operators theOp2;
      if ((theResult = parseOperator(theOp2))) {
        // merge two valid operator and check validity again
        anOp = Helpers::toOperator(theTokenOp1.data + theTokenOp2.data);
        if (Operators::unknown_op == anOp) {
          return {Errors::invalidOperator};
        }
      }
    }
  }
  return theResult;
}

// where operand is field, number, string...
auto ParseHelper::parseOperand(const Entity &anEntity, Operand &anOperand)
    -> StatusResult {
  StatusResult theResult{Errors::noError};
  Token &theToken = tokenizer.current();
  if (TokenType::identifier == theToken.type) {
    if (auto *theAttr = anEntity.getAttribute(theToken.data)) {
      anOperand.ttype = theToken.type;
      anOperand.name = theToken.data;  // hang on to name...
      anOperand.entityId = Helpers::hashString(anEntity.getName());
      anOperand.dtype = theAttr->getType();
    } else {
      anOperand.ttype = TokenType::string;
      anOperand.dtype = DataTypes::varchar_type;
      anOperand.value = theToken.data;
    }
  } else if (TokenType::number == theToken.type) {
    anOperand.ttype = TokenType::number;
    anOperand.dtype = DataTypes::int_type;
    if (theToken.data.find(dot) != std::string::npos) {
      anOperand.dtype = DataTypes::float_type;
      anOperand.value = std::stof(theToken.data);
    } else {
      anOperand.value = std::stoi(theToken.data);
    }
  } else {
    theResult.error = Errors::syntaxError;
  }
  if (theResult) {
    tokenizer.next();
  }
  return theResult;
}

}  // namespace ECE141
