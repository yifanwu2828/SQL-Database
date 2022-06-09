/**
 * @file FolderReader.cpp
 * @author Yifan Wu
 * @brief
 * @version 0.4
 * @date 2022-04-22
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "FolderReader.hpp"

#include <filesystem>

namespace ECE141 {
FolderReader::FolderReader(const std::string &aPath)
    : path{aPath}, directory{fs::path(aPath)} {}

bool FolderReader::exists(const std::string &aFilename) const {
  return fs::exists(directory / aFilename);
}

bool FolderReader::remove(const std::string &aFilename) const {
  // true if the file was deleted, false if it did not exist.
  //  file is reading by a stream -> fail to remove
  return fs::remove(directory / aFilename);
}

void FolderReader::each(const std::string &anExt,
                        const FileVisitor &aVisitor) const {
  // iterate through all file in director path
  for (const auto &entry : fs::directory_iterator(directory)) {
    if (!entry.is_directory() && fileExtensionIs(entry, anExt)) {
      if (!aVisitor(entry.path().stem().string())) {
        break;
      }
    }
  }
}

bool FolderReader::fileExtensionIs(const fs::directory_entry &entry,
                                   const std::string &anExt) {
  return entry.path().extension() == anExt;
}

}  // namespace ECE141