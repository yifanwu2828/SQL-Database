/**
 * @file LRUCache.hpp
 * @author Yifan Wu
 * @brief
 * @version 10
 * @date 2022-05-31
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef LRUCache_h
#define LRUCache_h

#include <string>
#include <utility>
#include <map>
#include <deque>
#include <algorithm>

#include "BasicTypes.hpp"
#include "Errors.hpp"
#include "Filters.hpp"
#include "keywords.hpp"

namespace ECE141 {

template <typename KeyT, typename ValueT>
class LRUCache {
 public:
  // OCF

  void put(const KeyT& key, const ValueT& value) {
      if(contains(key)) {return;}
      
      if(size() == maxsize) {
          manageFullCache(key);
      }
      else if(size() > maxsize) {
          std::cout << "cache too big" << std::endl;
      }
      cacheMap.insert({key, value});
      cacheOrder.push_front(key);
  }
  ValueT& get(const KeyT& key) {
      return cacheMap[key];
  }
  bool contains(const KeyT& key) const {
      if(cacheMap.find(key) != cacheMap.end()) {
          return true;
      }
      return false;
  }
  size_t size() const { // current size
      return cacheMap.size();
  }
    
  void manageFullCache(const KeyT key) {
      if(contains(key)) {
          cacheOrder.erase(std::find(cacheOrder.begin(), cacheOrder.end(), key));
      }
      else {
          cacheMap.erase(cacheMap.find(cacheOrder.back()));
          cacheOrder.pop_back();
      }
  }
    
  void deleteKey(const KeyT key) {
      if(contains(key)) {
          cacheMap.erase(cacheMap.find(key));
          cacheOrder.erase(std::find(cacheOrder.begin(), cacheOrder.end(), key));
      }
  }
    
 void setMaxsize(const size_t aSize) {maxsize = aSize;}

 protected:
  size_t maxsize;  // prevent cache from growing past this size...

  // data members here...
  std::deque<KeyT> cacheOrder;
  std::map<KeyT, ValueT> cacheMap;
};

}  // namespace ECE141

#endif /* LRUCache_h */
