/**
 * @file TabularView.hpp
 * @author Yifan Wu
 * @brief 
 * @version 0.1
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#ifndef TabularView_h
#define TabularView_h

#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "BasicTypes.hpp"
#include "Config.hpp"
#include "Helpers.hpp"
#include "Row.hpp"
#include "View.hpp"

namespace ECE141 {

// USE: general tabular view (with columns)
class TabularView : public View {
 public:
  TabularView(std::ostream &anOutput, RowCollection &aRows,
              StringList aFieldList)
      : output{anOutput}, rows{aRows}, fieldList{std::move(aFieldList)} {
    if (!fieldList.empty()) {
      if ("*" == fieldList[0]) {
        fieldList.clear();
        for (const auto &[theAttr, theValue] : rows.front()->getData()) {
          fieldList.push_back(theAttr);
        }
      }
      std::sort(fieldList.begin(), fieldList.end(),
                [](const auto &a, const auto &b) {
                  if (a == "id") {
                    return true;
                  }
                  if (b == "id") {
                    return false;
                  }
                  return a < b;
                });
      const auto &theData = rows[0]->getData();
      std::transform(
          fieldList.begin(), fieldList.end(), std::back_inserter(widths),
          [&](const auto &theAttr) -> std::streamsize {
            return std::max(static_cast<std::streamsize>(theAttr.size() + 2),
                            Helpers::getWidthFromVal(theData.at(theAttr)));
          });
    }
  }

  // USE: create header for tabular view...
  TabularView &showHeader() {
    printBreak();
    output.fill(' ');
    output.setf(std::ios::left, std::ios::adjustfield);

    for (size_t i = 0; i < fieldList.size(); i++) {
      const auto &theAttr = fieldList[i];
      output.width(widths[i]);
      output << "| " + theAttr;
      output << " ";
    }

    output << "|\n";
    printBreak();

    return *this;
  }

  TabularView &showFooter() {
    printBreak();
    output << rows.size() << " rows in set (" << Config::getTimer().elapsed()
           << " sec)\n";
    return *this;
  }

  TabularView &showFooter(size_t aCount, double anElapsed) {
    output << Helpers::QueryOk(aCount, anElapsed);
    return *this;
  }

  // USE: this function shows all the fields in a row...
  TabularView &showRow(Row &aRow) {
    const auto &theData = aRow.getData();
    for (size_t i = 0; i < fieldList.size(); i++) {
      const auto &attrName = fieldList[i];
      output.width(widths[i]);
      if (theData.find(attrName) != theData.end()) {
        std::string theStr = Helpers::valToString(theData.at(attrName));
        theStr.pop_back();
        output << "| " + theStr << " ";
      } else {
        // std::cerr << "| Unknow" + attrName << '\n';
        output << "| Unknow" + attrName << " ";
      }
    }
    output << "|\n";
    return *this;
  }

  void printBreak() {
    if (fieldList.empty()) {
      return;
    }
    output << '+';
    for (size_t i = 0; i < fieldList.size(); i++) {
      output.width(widths[i]);
      output.fill('-');
      output << '-';
      output << '+';
      output.fill(' ');
    }
    output << "\n";
  }

  // USE: this is the main show() for the view, where it presents all the
  // rows...
  // TODO: show not implement just fullfil the virtual base class
  bool show([[maybe_unused]] std::ostream &anOutput) const override {
    return true;
  }

  bool show() {
    showHeader();
    for (const auto &theRow : rows) {
      showRow(*theRow);
    }
    showFooter();
    return true;
  }

 protected:
  // Entity              &entity; //if necessary?
  std::ostream &output;
  RowCollection &rows;
  StringList fieldList;
  std::string separator;
  std::vector<std::streamsize> widths;
};

}  // namespace ECE141

#endif /* TabularView_h */
