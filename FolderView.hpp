/**
 * @file FolderView.hpp
 * @author Yifan Wu
 * @brief
 * @version 0.4
 * @date 2022-04-18
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef FolderView_h
#define FolderView_h

#include <string>
#include <ostream>
#include "FolderReader.hpp"
#include "View.hpp"

namespace ECE141 {

// USE: provide view class that lists db files in storage path...
class FolderView : public View {
 public:
  explicit FolderView(const std::string &aPath,
                      const char *anExtension = ".db");

  bool show(std::ostream &anOutput) const override;
  
 protected:
  FolderReader reader;
  const char *extension;
  friend class Application;
};

}  // namespace ECE141

#endif /* FolderView_h */
