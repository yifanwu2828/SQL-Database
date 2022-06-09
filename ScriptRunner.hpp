/**
 * @file ScriptRunner.hpp
 * @author Yifan Wu
 * @brief 
 * @version 0.9
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef ScriptRunner_h
#define ScriptRunner_h

#include <iosfwd>
#include <string>



namespace ECE141 {
class Application;
struct Color {
  std::string RESET = "\033[0m";
  std::string RED = "\033[31m";   /* Red */
  std::string GREEN = "\033[32m"; /* Green */

  [[nodiscard]] std::string toRed(const std::string &aStr) const;
  [[nodiscard]] std::string toGreen(const std::string &aStr) const;
};

class StatusResult;
class ScriptRunner {
 public:
  explicit ScriptRunner(Application &anApp);
  static void showErrors(StatusResult &aResult, std::ostream &anOutput);
  StatusResult run(std::istream &anInput, std::ostream &anOutput);

 protected:
  Application &app;
  Color color;
};

}  // namespace ECE141
#endif /* ScriptRunner_h */