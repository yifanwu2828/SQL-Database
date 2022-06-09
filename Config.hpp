/**
 * @file Config.hpp
 * @author Yifan Wu
 * @brief 
 * @version 10
 * @date 2022-05-31
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef Config_h
#define Config_h

#include <array>
#include <filesystem>
#include <sstream>
#include <string>

#include "Timer.hpp"

namespace ECE141 {

enum class CacheType : int { block = 0, rows = 1, views = 2 };

struct Config {

  static std::array<size_t,3> cacheSize;
  
  static const char* getDBExtension() { return ".db"; }

  static std::string getStoragePath() {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

    return std::filesystem::temp_directory_path().string();

#elif __APPLE__ || defined __linux__ || defined __unix__

    return {"/tmp"};

#endif
  }

  static Timer& getTimer() {
    static Timer theTimer;
    return theTimer;
  }

  static std::string getDBPath(const std::string& aDBName) {
    std::ostringstream theStream;
    theStream << Config::getStoragePath() << '/' << aDBName << ".db";
    return theStream.str();
  }

  // simply  "foo" -> "foo.db"
  static std::string getDBFilename(const std::string& aDBName) {
    return aDBName + ".db";
  }

  static std::string getVersion() { return {"0.9"}; }

  static size_t getCacheSize(CacheType aType) {
    return cacheSize.at(static_cast<int>(aType));
  }

  static void setCacheSize(CacheType aType, size_t aSize) {
    cacheSize.at(static_cast<int>(aType)) = aSize;
  }
  
  //cachetype: block, row, view...
  static bool useCache(CacheType aType) {
    return cacheSize.at(static_cast<int>(aType)) > 0;
  }	

  static bool useIndex() { return true; }
};

}  // namespace ECE141

#endif /* Config_h */