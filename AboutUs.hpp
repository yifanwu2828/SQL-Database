/**
 * @file AboutUs.hpp
 * @author Yifan Wu, Samuel Woo
 * @brief
 * @version 0.1
 * @date 2022-04-22
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef AboutMe_hpp
#define AboutMe_hpp

#include <optional>
#include <string>
#include <vector>

namespace ECE141 {

using StringOpt = std::optional<std::string>;

class AboutUs {
 public:
  AboutUs() = default;
  ~AboutUs() = default;

  static size_t getTeamSize() { return 2; }  // STUDENT: update this...

  static StringOpt getName(size_t anIndex) {
    // return name of student anIndex N (or nullopt)
    std::vector<std::string> theName = {"Yifan Wu", "Samuel Woo"};
    return theName[anIndex];
  }
  static StringOpt getGithubUsername(size_t anIndex) {
    // return github username of student anIndex N (or nullopt)
    std::vector<std::string> theUserName = {"yifanwu2828", "samuel-d-woo"};
    return theUserName[anIndex];
  }
};

}  // namespace ECE141

#endif /* about_me */
