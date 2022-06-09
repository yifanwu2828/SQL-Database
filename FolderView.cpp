/**
 * @file FolderView.cpp
 * @author Yifan Wu
 * @brief
 * @version 0.9
 * @date 2022-04-22
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "FolderView.hpp"

#include <cstddef>
#include <vector>

#include "TableFormatter.hpp"

namespace ECE141 {
FolderView::FolderView(
    const std::string &aPath,
    const char *anExtension)  // extension should included "."
    : reader{aPath}, extension{anExtension} {}

bool FolderView::show(std::ostream &anOutput) const {
  std::string theExtension{extension};
  size_t theCount{0};
  std::vector<std::streamsize> theWidths{23};
  TableFormatter::printBreak(anOutput, theWidths);
  reader.each(theExtension, [&](const std::string &aName) -> bool {
    anOutput.width(theWidths[0]);
    anOutput << ("| " + aName) << "|\n";
    theCount++;
    return true;
  });
  TableFormatter::printBreak(anOutput, theWidths);
  TableFormatter::printRowsInSet(anOutput, theCount);
  return true;
}

}  // namespace ECE141