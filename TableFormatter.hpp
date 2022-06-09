/**
 * @file TableFormatter.hpp
 * @author Sam Woo
 * @brief
 * @version 0.3
 * @date 2022-04-18
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef TableFormatter_h
#define TableFormatter_h

#include <array>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Errors.hpp"
#include "View.hpp"

namespace ECE141 {

constexpr const int DEFAULT_PRECISION{6};

class TableFormatter : public View {
 public:
  TableFormatter() = default;

  // TODO: show not implement
  bool show([[maybe_unused]] std::ostream &anOutput) const override {
    return true;
  }

  static void printBreak(std::ostream &anOutput,
                         const std::vector<std::streamsize> &aWidths) {
    anOutput << '+';
    for (const auto &aWidth : aWidths) {
      anOutput.width(aWidth - 1);
      anOutput.fill('-');
      anOutput << '-';
      anOutput << '+';
      anOutput.fill(' ');
    }
    anOutput << "\n";
  }

  static void printRowsInSet(std::ostream &anOutput, size_t aCount) {
    anOutput << aCount << " rows in set ";
  }

  static void printDuration(std::ostream &anOutput, const double aDuration) {
    anOutput << "(" << std::setprecision(DEFAULT_PRECISION) << std::fixed << aDuration
             << " sec)"
             << "\n";
  }

  static void printStatus(std::ostream &anOutput, StatusResult aResult) {
    static const std::array<std::string, 2> status = {"FAIL", "OK"};
    anOutput << "Query " << status.at(static_cast<size_t>(aResult)) << ", ";
  }

  static void printRowsAffected(std::ostream &anOutput, size_t aCount) {
    anOutput << aCount << " rows affected ";
  }

  static void printStatusRowDuration(std::ostream &anOutput,
                                     StatusResult aResult, size_t aCount,
                                     const double aDuration) {
    printStatus(anOutput, aResult);
    printRowsAffected(anOutput, aCount);
    printDuration(anOutput, aDuration);
  }

  static void printDBChange(std::ostream &anOutput, bool aChange) {
    if (aChange) {
      anOutput << "Database changed \n";
    } else {
      anOutput << "Database not changed \n";
    }
  }
};

}  // namespace ECE141

#endif /* TableFormatter_h */
