/**
 * @file Compare.cpp
 * @author Yifan Wu
 * @brief 
 * @version 0.1
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "Compare.hpp"


bool isEqual(const std::string &aStr1, const std::string &aStr2) {
  return aStr1 == aStr2;
}

bool isEqual(const std::string &aStr, const bool &arg2) {
  return isEqual(arg2, aStr);
}

bool isEqual(const std::string &aStr, const int &arg2) {
  return isEqual(arg2, aStr);
}

bool isEqual(const std::string &aStr, const double &arg2) {
  return isEqual(arg2, aStr);
}

bool isLess(const std::string &aStr1, const std::string &aStr2) {
  return aStr1 < aStr2;
}

bool isLess(const std::string &aStr, const bool &arg2) {
  return isLess(arg2, aStr);
}

bool isLess(const std::string &aStr, const int &arg2) {
  return isLess(arg2, aStr);
}

bool isLess(const std::string &aStr, const double &arg2) {
  return isLess(arg2, aStr);
}