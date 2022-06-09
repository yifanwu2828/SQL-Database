/**
 * @file Tokenizer.hpp
 * @author Yifan Wu
 * @brief 
 * @version 0.9
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef Tokenizer_hpp
#define Tokenizer_hpp

#include <iosfwd>
#include <cstddef>
#include <vector>
#include <string>       

#include "Scanner.hpp"
#include "keywords.hpp"

namespace ECE141 {

class StatusResult;
using parseCallback = bool(char aChar);

enum class TokenType {
  function,
  identifier,
  keyword,
  number,
  operators,
  timedate,
  punctuation,
  string,
  unknown
};

//-----------------

bool isWhitespace(char aChar);
bool isNumber(char aChar);
bool isAlphaNum(char aChar);
bool isQuote(char aChar);
bool isOperator(char aChar);
bool isSign(char aChar);
bool isPunctuation(char aChar);
bool isUnknown(char aChar);
//-----------------------------------

struct Token {
  TokenType type;
  Keywords keyword;
  Operators op;
  std::string data;
};

//-----------------

class Tokenizer : public Scanner {
 public:
  explicit Tokenizer(std::istream& anInputStream);

  StatusResult tokenize();
  Token& tokenAt(size_t anOffset);

  Token& current();
  bool more() { return index < size(); }
  bool next(int anOffset = 1);
  Token& peek(int anOffset = 1);

  void restart();
  [[nodiscard]] size_t currentIndex() const;
  size_t size() const;
  size_t remaining() const;

  // these might consume a token...
  bool skipTo(Keywords aKeyword);
  bool skipTo(TokenType aTokenType);

  bool skipIf(Keywords aKeyword);
  bool skipIf(Operators anOperator);
  bool skipIf(TokenType aTokenType);
  bool skipIf(char aChar);

  void dump();  // utility

 protected:
  std::vector<Token> tokens;
  size_t index;
};

}  // namespace ECE141

#endif /* Tokenizer_hpp */
