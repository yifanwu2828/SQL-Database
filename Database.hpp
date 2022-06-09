/**
 * @file Database.hpp
 * @author Yifan Wu
 * @brief
 * @version 0.9
 * @date 2022-05-27
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef Database_hpp
#define Database_hpp

#include <iosfwd>
#include <string>

#include "Index.hpp"
#include "Storage.hpp"
#include "Joins.hpp"    // for Join
#include "Row.hpp"      // for Row

namespace ECE141 {
class DBQuery;
class StatusResult;

struct CreateDB {};  // tags for db-open modes...
struct OpenDB {};

class Database {
 public:
  Database(const std::string &aPath, CreateDB);
  Database(const std::string &aPath, OpenDB);
  ~Database();

  Database(const Database &) = delete;
  Database &operator=(const Database &) = delete;
  Database(Database &&) = delete;
  Database &operator=(Database &&) = delete;

  // setters
  void setChanged(bool aChange);
  void setDebugInfo(std::string &&anInfo);
  bool entityExistsInDB(const std::string &aName) const;

  // getters
  std::string getName() const;
  Storage &getStorage();
  Index &getEntityIndex();
  IndexMap &getIndexMap();

  // DB level dump info for debug
  StatusResult dump(std::ostream &anOutput);
  StatusResult selectRow(const DBQuery &aQuery, RowCollection &aCollection);
  StatusResult updateRow(const DBQuery &aQuery, RowCollection &aCollection);
  StatusResult deleteRow(const DBQuery &aQuery, RowCollection &aCollection);

 protected:
  std::string name;  // name of database
  bool changed;      // indicate save operation required before destruct
  Storage storage;   // storage interface to save and load block
  Index entityIndex;
  IndexMap indexMap;
  std::string debugInfo;  // debug message

  StatusResult getAllRowsFrom(const std::string &aTableName,
                              RowCollection &aCollection);
  StatusResult joinRows(const JoinList &aJoinList, RowCollection &aCollection);
};

// Free Functions
StatusResult leftJoinRows(const Join &aJoin,
                          const RowCollection &aLHSCollection,
                          const RowCollection &aRHSCollection,
                          RowCollection &aJoinedCollection);

StatusResult rightJoinRows(const Join &aJoin,
                           const RowCollection &aLHSCollection,
                           const RowCollection &aRHSCollection,
                           RowCollection &aJoinedCollection);

StatusResult innerJoinRows(const Join &aJoin,
                           const RowCollection &aLHSCollection,
                           const RowCollection &aRHSCollection,
                           RowCollection &aJoinedCollection);

StatusResult crossJoinRows(const Join &aJoin,
                           const RowCollection &aLHSCollection,
                           const RowCollection &aRHSCollection,
                           RowCollection &aJoinedCollection);

StatusResult fullJoinRows(const Join &aJoin,
                          const RowCollection &aLHSCollection,
                          const RowCollection &aRHSCollection,
                          RowCollection &aJoinedCollection);

}  // namespace ECE141
#endif /* Database_hpp */
