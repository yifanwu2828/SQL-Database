/**
 * @file FolderReader.hpp
 * @author Yifan Wu
 * @brief
 * @version 0.4
 * @date 2022-04-18
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef FolderReader_h
#define FolderReader_h

#include <filesystem>
#include <functional>
#include <string>

namespace fs = std::filesystem;

namespace ECE141 {

using FileVisitor = std::function<bool(const std::string &)>;

class FolderReader {
 public:
  explicit FolderReader(const std::string &aPath);

  virtual ~FolderReader() = default;
  [[nodiscard]] virtual bool exists(const std::string &aFilename) const;
  [[nodiscard]] virtual bool remove(const std::string &aFilename) const;

  virtual void each(const std::string &anExt,
                    const FileVisitor &aVisitor) const;

  static bool fileExtensionIs(const fs::directory_entry &entry,
                              const std::string &anExt);

 protected:
  std::string path;  // path = "/tmp"; from Config::getStoragePath();
  fs::path directory;
};

}  // namespace ECE141

#endif /* FolderReader_h */
