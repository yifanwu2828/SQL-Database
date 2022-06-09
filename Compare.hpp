/**
 * @file Compare.hpp
 * @author Yifan Wu
 * @brief 
 * @version 0.9
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef compare_h
#define compare_h

#include <sstream>
#include <string>

// Equal ------------------------------------------
template <typename T>
bool isEqual(const T &arg1, const T &arg2) {
  return arg1 == arg2;
}

bool isEqual(const std::string &aStr1, const std::string &aStr2);

template <typename T1, typename T2>
bool isEqual(const T1 &arg1, const T2 &arg2) {
  return static_cast<T1>(arg2) == arg1;
}

template <typename T1>
bool isEqual(const T1 &arg1, const std::string &aStr) {
  std::stringstream temp;
  temp << arg1;
  std::string theStr = temp.str();
  bool theResult = theStr == aStr;
  return theResult;
}

bool isEqual(const std::string &aStr, const bool &arg2);
bool isEqual(const std::string &aStr, const int &arg2);
bool isEqual(const std::string &aStr, const double &arg2);

// Less than ------------------------------------------
template <typename T>
bool isLess(const T &arg1, const T &arg2) {
  return arg1 < arg2;
}

bool isLess(const std::string &aStr1, const std::string &aStr2);

template <typename T1, typename T2>
bool isLess(const T1 &arg1, const T2 &arg2) {
  return arg1 < static_cast<T1>(arg2);
}

template <typename T1>
bool isLess(const T1 &arg1, const std::string &aStr) {
  std::stringstream temp;
  temp << arg1;
  std::string theStr = temp.str();
  bool theResult = theStr >= aStr;
  return theResult;
}

bool isLess(const std::string &aStr, const bool &arg2);
bool isLess(const std::string &aStr, const int &arg2);
bool isLess(const std::string &aStr, const double &arg2);
//--------- feel free to add more operators... ------

#endif /* compare_h */
