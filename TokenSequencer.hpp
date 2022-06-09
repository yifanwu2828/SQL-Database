/**
 * @file Sequence.hpp
 * @author Yifan Wu
 * @brief
 * @version 0.3
 * @date 2022-04-16
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef Sequence_hpp
#define Sequence_hpp

#include <initializer_list>
#include <string>
#include <map>
#include <vector> 

#include "Attribute.hpp"
#include "BasicTypes.hpp"
#include "Entity.hpp"
#include "Tokenizer.hpp"
#include "keywords.hpp"

namespace ECE141 {

using KWList = std::initializer_list<Keywords>;

class TokenSequencer {
 public:
  explicit TokenSequencer(Tokenizer &aTokenizer) : tokenizer(aTokenizer), state{true} {}

  struct SeqState {
    bool state;
    TokenSequencer &seq;
  };

  TokenSequencer &parseAttributes(Entity &anEntity) {
    if (state) {
      static std::map<Keywords, DataTypes> theDataTypesMap = {
          {Keywords::integer_kw, DataTypes::int_type},
          {Keywords::float_kw, DataTypes::float_type},
          {Keywords::boolean_kw, DataTypes::bool_type},
          {Keywords::varchar_kw, DataTypes::varchar_type},
          {Keywords::datetime_kw, DataTypes::datetime_type}};

      Attribute theAttr;
      if (TokenType::identifier == tokenizer.current().type) {
        theAttr.setName(tokenizer.current().data);
        tokenizer.next();
        if (theDataTypesMap.find(tokenizer.current().keyword) !=
            theDataTypesMap.end()) {
          theAttr.setDataType(theDataTypesMap[tokenizer.current().keyword]);
          tokenizer.next();
          if (theAttr.getType() == DataTypes::varchar_type) {
            tokenizer.next();
            int theSize;
            getNumber(theSize);
            theAttr.setSize(theSize);
            tokenizer.next(2);
          }
          while (state && tokenizer.current().type != TokenType::punctuation &&
                 tokenizer.more()) {
            if (currentIs({Keywords::not_kw, Keywords::null_kw})) {
              theAttr.setNullable(false);
              state = true;
            } else if (clear().currentIs({Keywords::auto_increment_kw})) {
              theAttr.setAutoIncrement(true);
              state = true;
            } else if (clear().currentIs(
                           {Keywords::primary_kw, Keywords::key_kw})) {
              theAttr.setPrimaryKey(true);
              state = true;
            } else if (clear().currentIs({Keywords::default_kw})) {
              theAttr.setDefaultVal(theAttr.toValue(tokenizer.current().data));
              tokenizer.next();
              state = true;
            } else {
              state = false;
            }
          }
          if (theAttr.isPrimaryKey()) {
            theAttr.setNullable(false);
          }
          anEntity.addAttribute(theAttr);
        }
      }
    }
    return *this;
  }
    
  TokenSequencer &parseAttributeList(StringList &aAttrList) {
      if(state) {
          bool hasComma = true;
          while(hasComma && tokenizer.current().type != TokenType::punctuation) {
              Token &theToken = tokenizer.current();
              if(theToken.type == TokenType::identifier) {
                  aAttrList.push_back(theToken.data);
                  tokenizer.next();
                  hasComma = tokenizer.skipIf(',');
                  theToken = tokenizer.current();
              }
          }
      }
      return *this;
  }

  // ? new ------------------------------------------------------------------------------- 
  TokenSequencer &parseValuesList(ValuesList &aValList) {
      if(state) {
          Token &theToken = tokenizer.current();
          while (theToken.type != TokenType::punctuation && tokenizer.more()) {
              if(theToken.data.compare(theToken.data.size()-1, 1, ",") == 0) { //check if the data ends with a comma, if so remove it
                  theToken.data.pop_back();
              }
              aValList.push_back(theToken.data);
              tokenizer.next();
              tokenizer.skipIf(',');
              theToken = tokenizer.current();
          }
          state = tokenizer.skipIf(')') && tokenizer.skipIf(',') && tokenizer.skipIf('(');
      }
      return *this;
  }

  TokenSequencer &currentIs(const KWList &aList) {
    if (state) {
      std::vector<Keywords> theList{aList};
      int thePos{0};
      for (auto theKW : theList) {
        // this actually check pos then pos++
        Token &theToken = tokenizer.peek(thePos++);
        if (theKW != theToken.keyword) {
          state = false;
          break;
        }
      }
      if (state) {
        return skip(theList.size());
      }
    }
    return *this;
  }

  TokenSequencer &currentIsNoSkip(const KWList &aList) {
    if (state) {
      std::vector<Keywords> theList{aList};
      int thePos{0};
      for (auto theKW : theList) {
        Token &theToken = tokenizer.peek(thePos++);
        if (theKW != theToken.keyword) {
          state = false;
          break;
        }
      }
      // if (state) {
      //   return skip(theList.size());
      // }
    }
    return *this;
  }

  TokenSequencer &getNumber(int &aValue) {
    const Token &theToken = tokenizer.current();
    aValue = std::stoi(theToken.data);
    return *this;
  }

  TokenSequencer &skipPast(char aChar) {
    if (state) {
      while (tokenizer.next()) {
        auto &theToken = tokenizer.current();
        if (theToken.type == TokenType::punctuation) {
          if (theToken.data[0] == aChar) {
            tokenizer.next();  // skip it...
            return *this;
          }
        }
      }
      state = false;
    }
    return *this;
  }

  TokenSequencer &skip(size_t aCount) {
    if (state) {
      tokenizer.next((int)aCount);  // actually skip...
    }
    return *this;
  }

  operator bool() const { return state; }

  TokenSequencer &clear() {
    state = true;
    return *this;
  }

 protected:
  Tokenizer &tokenizer;
  bool state;
};

}  // namespace ECE141

#endif /*Sequence.hpp */
