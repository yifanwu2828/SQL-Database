/**
 * @file Storage.hpp
 * @author Yifan Wu
 * @brief 
 * @version 0.9
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef Storage_hpp
#define Storage_hpp

#include <cstddef>
#include <cstdint>  // for uint32_t, int32_t
#include <deque>
#include <functional>
#include <ios>  // for ios_base, ios_base::openmode
#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "BlockIO.hpp"

namespace ECE141 {

class Entity;
class Index;
class Row;
class StatusResult;
using RowCollection = std::vector<std::unique_ptr<Row>>;
using IndexMap = std::map<std::string, std::unique_ptr<Index>>;

class Storable {
 public:
  virtual ~Storable() = default;
  virtual StatusResult encode(std::ostream &anOutput) const = 0;
  virtual StatusResult decode(std::istream &anInput) = 0;
};

// ---------------------------------------------------------
struct StorageInfo {
  StorageInfo() = default;

  StorageInfo(size_t aRefId, size_t aSize, int32_t aStartPos = kNewBlock,
              BlockType aType = BlockType::data_block,
              std::string anExtra = "");

  StorageInfo(size_t aRefId, std::streampos aSize,
              int32_t aStartPos = kNewBlock,
              BlockType aType = BlockType::data_block,
              std::string anExtra = "");

  explicit StorageInfo(const BlockHeader &aHeader);
  StorageInfo &operator=(const BlockHeader &aHeader);

  friend std::ostream &operator<<(std::ostream &aStream,
                                  const StorageInfo &anInfo);

  std::string extra;
  size_t refId{0};
  size_t size{0};            // actual size of payload (not maximum size)
  int32_t start{kNewBlock};  // blockNum idx
  BlockType type{BlockType::unknown_block};
};
StorageInfo getMetaStorageInfo(std::streampos aSize);
StorageInfo getLookUpStorageInfo(std::streampos aSize);
// ---------------------------------------------------------

using BlockVisitor = std::function<bool(const Block &, uint32_t)>;
using BlockList = std::deque<uint32_t>;

struct BlockIterator {
  virtual bool each(BlockVisitor) = 0;
};
// ---------------------------------------------------------
// USE: Our storage manager class...
class Storage : public BlockIO, public BlockIterator {
 public:
  Storage(const std::string &aPath, const std::ios_base::openmode &aMode);

  StatusResult save(std::iostream &aStream, StorageInfo &anInfo,
                    const std::string &aDBName = "");

  StatusResult load(std::iostream &aStream, StorageInfo &anInfo,
                    uint32_t aStartBlockNum, const std::string &aDBName = "");

  bool each(BlockVisitor aVisitor) override;

  static StatusResult encodeIndexMap(std::ostream &anOutput,
                                     const IndexMap &anIndexMap);
  StatusResult decodeIndexMap(std::istream &anInput, IndexMap &anIndexMap);

 protected:
  // Save and load Index/Meta Block
  StatusResult saveMetaBlock(const Index &anIndex);
  StatusResult loadMetaBlock(Index &anIndex);

  StatusResult saveIndexMap(const IndexMap &anIndexMap);
  StatusResult loadIndexMap(IndexMap &anIndexMap);

  StatusResult saveEntityBlock(const Entity &anEntity,
                               int32_t BlockNum = kNewBlock);
  StatusResult saveDataBlock(const Row &aRow, int32_t BlockNum = kNewBlock);

  // Block Related
  StatusResult markBlockAsFree(uint32_t aPos);
  StatusResult releaseBlocks(uint32_t aPos, bool aInclusive = false);
  uint32_t getFreeBlock();  // pos of next free (or new)...

  // ----------------------------------------------
  // get/drop Row by iter all data Block
  StatusResult getRowsByBruteForce(const Entity &anEntity,
                                   RowCollection &aCollection);
  StatusResult getRowsByBruteForce(const std::string &anEntityName,
                                   RowCollection &aCollection);
  StatusResult dropRowsByBruteForce(const std::string &anEntityName);

  // ----------------------------------------------
  // get/drop Row by using index
  StatusResult getRowsByIndex(const std::string &anEntityName,
                              const IndexMap &anIndexMap,
                              RowCollection &aCollection);
  StatusResult dropRowsByIndex(const std::string &anEntityName,
                               IndexMap &anIndexMap);

  BlockList available;
  friend class Database;
  friend class DBProcessor;
};

}  // namespace ECE141

#endif /* Storage_hpp */
