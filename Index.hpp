/**
 * @file Index.hpp
 * @author Yifan Wu
 * @brief 
 * @version 0.9
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef Index_hpp
#define Index_hpp

#include <cstdint>
#include <cstddef>

#include <functional>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <variant>
// #include <vector>

#include "BasicTypes.hpp"
#include "Storage.hpp"

namespace ECE141 {
class StatusResult;

enum class IndexType { intKey = 'I', strKey = 'S' };
using IndexKey = std::variant<uint32_t, std::string>;

using IndexVisitor = std::function<bool(const IndexKey &, uint32_t)>;

class Index : public Storable, BlockIterator {
 public:
  explicit Index(Storage &aStorage, uint32_t aBlockNum = 0,
                 IndexType aType = IndexType::intKey,
                 const std::string &aName = "Null");

  class ValueProxy {
   public:
    Index &index;
    IndexKey key;
    IndexType type;

    ValueProxy(Index &anIndex, uint32_t aKey);
    ValueProxy(Index &anIndex, const std::string &aKey);

    ValueProxy &operator=(uint32_t aValue);
    explicit operator IntOpt() const;
  };  // value proxy

  ValueProxy operator[](const std::string &aKey);
  ValueProxy operator[](uint32_t aKey);

  // setter
  Index &setChanged(bool aChanged);
  Index &setIndexBlockNum(uint32_t aBlockNum);
  Index &setName(std::string aName);
  bool setKeyValue(const IndexKey &aKey, uint32_t aValue);

  // getter
  [[nodiscard]] uint32_t getBlockNum() const;
  [[nodiscard]] std::string getName() const;
  [[nodiscard]] uint32_t getEntityId() const;
  [[nodiscard]] size_t getSize() const;
  [[nodiscard]] IndexType getType() const;

  [[nodiscard]] StorageInfo getStorageInfo(
      size_t aSize, const std::string &aTableName = "") const;

  [[nodiscard]] bool isChanged() const;
  [[nodiscard]] bool isEmpty() const;
  [[nodiscard]] bool exists(const IndexKey &aKey) const;

  [[nodiscard]] IntOpt valueAt(const IndexKey &aKey) const;
  StatusResult erase(uint32_t aKey);
  StatusResult erase(const std::string &aKey);

  // Storable interface
  StatusResult encode(std::ostream &anOutput) const override;
  StatusResult decode(std::istream &anInput) override;

  // visit blocks associated with index
  bool each(BlockVisitor aVisitor) override;
  // visit index values (key, value)...
  bool eachKV(const IndexVisitor &aCall);

  // ? custom
  // --------------------------------------------------------------------

  static void encodeIndexKey(std::ostream &anOutput, const IndexKey &anIndexKey,
                             const IndexType &anIdxType);

  static IndexKey toIndexKey(const std::string &aStr, IndexType anIdxType);

  static IndexKey valueToIndexKey(Value theVal, DataTypes aDType) {
    if (DataTypes::int_type == aDType) {
      return static_cast<uint32_t>(std::get<int>(theVal));
    }
    return std::get<std::string>(theVal);
  }

 protected:
  Storage &storage;
  std::map<IndexKey, uint32_t> data;  //  IndexKey of data : blockNum of a Row?
  std::string name{"Null"};           // attrName
  uint32_t entityId{0};               // ? Hash
  uint32_t blockNum{0};  // index block's blkNum (where index storage begins)
  IndexType type;
  bool changed{false};
};
// table name : index or vector<index>
// using IndexMap = std::map<std::string, std::vector<std::unique_ptr<Index>>>;
using IndexMap = std::map<std::string, std::unique_ptr<Index>>;

// create table ->
//        - entity blk
//        - create index object combine
//        - index block (obtain block number, write header)
//        - create index object(set blockNum to this object)
//        - indexMap[table-name] = index;

// meta 0  user 1 (entity blockNum)
// meta 1  user 2(index)

// insert statement : table-name
//        - find table-name -> index's data[primary key] = storageInfo.start
//        - changed = true; call index.encode(block num)

// use foo
//        -load entity + load indexMap
//

// select
//        - indexMap[tableName] -> Index.map
//        where contains primarykey(index name)
//    if no id: find all pair in map take pair.second(blkNum)
//    if id:  select * from users where id = 10
//    map[10] -> block number

// drop
// mark index block as free

// delete
// remove associate primaryKey in index (IndexMap[tableName]) {val of primaryKey
// : blkNum}

}  // namespace ECE141

#endif /* Index_hpp */
