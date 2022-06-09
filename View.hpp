/**
 * @file View.hpp
 * @author Yifan Wu
 * @brief 
 * @version 0.9
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef View_h
#define View_h

#include <iosfwd>

namespace ECE141 {

// completely generic view, which you will subclass to show information
class View {
 public:
  virtual ~View() =default;
  virtual bool show(std::ostream &aStream) const = 0;
};

}  // namespace ECE141

#endif /* View_h */
