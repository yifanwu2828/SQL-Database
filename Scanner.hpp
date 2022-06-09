/**
 * @file Scanner.hpp
 * @author Yifan Wu
 * @brief 
 * @version 0.9
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef Scanner_h
#define Scanner_h

#include <iostream>

namespace ECE141 {

using parseCallback = bool(char aChar);

constexpr const char left_paren = '(';
constexpr const char right_paren = ')';
constexpr const char left_bracket = '[';
constexpr const char right_bracket = ']';
constexpr const char left_brace = '{';
constexpr const char right_brace = '}';
constexpr const char colon = ':';
constexpr const char comma = ',';
constexpr const char dot = '.';
constexpr const char semicolon = ';';
constexpr const char apostrophe = '\'';
constexpr const char quote = '"';

class Scanner {
 public:
  //----------------- simple parsing utilities ----------------------

  explicit Scanner(std::istream &anInput) : input(anInput) {}

  bool eof() { return input.eof(); }

  std::string readWhile(parseCallback aCallback) {
    std::string theResult;
    while (!input.eof() && (*aCallback)(input.peek())) {
      char theChar = input.get();
      theResult += theChar;
    }
    return theResult;
  }

  std::string readUntil(parseCallback aCallback, bool addTerminal = true) {
    std::string theResult;
    while (!input.eof() && !(*aCallback)(input.peek())) {
      theResult += input.get();
    }
    if (addTerminal) {
      if (!input.eof()) {
        theResult += input.get();
      }
    }
    return theResult;
  }

  std::string readUntil(char aTerminalChar, bool addTerminal = true) {
    std::string theResult;
    while (!input.eof()) {
      char theChar = input.get();
      if (theChar != aTerminalChar) {
        theResult += theChar;
      } else
        break;
    }
    if (addTerminal) {
      if (!input.eof()) {
        theResult += input.get();
      }
    }
    return theResult;
  }

  // skip chars as long as callback returns true
  bool skipWhile(parseCallback aCallback) {
    while (!input.eof() && (*aCallback)(input.peek())) {
      input.get();
    }
    return true;
  }

  // if the char is next in stream, skip it...
  bool skipIfChar(char aChar) {
    return (aChar == input.peek()) ? aChar == input.get() : false;
  }

 protected:
  std::istream &input;
};

}  // namespace ECE141

#endif /* Scanner_h */
