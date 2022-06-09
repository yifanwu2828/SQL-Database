/**
 * @file BlockIO.hpp
 * @author Yifan Wu
 * @brief 
 * @version 0.9
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef BlockIO_hpp
#define BlockIO_hpp

#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <map>
#include <string>
#include <string_view>

#include "LRUCache.hpp"

namespace ECE141 {

class StatusResult;

constexpr const int32_t kNewBlock = -1;
constexpr const int32_t META_BLOCK_NUM = 0;
constexpr const int32_t LOOKUP_BLOCK_NUM = 1;

constexpr const std::string_view kMetaBlockHash{"#Meta#"};
constexpr const std::string_view kLookupBlockHash{"#Lookup#"};

enum class BlockType {
  data_block = 'D',
  entity_block = 'E',
  free_block = 'F',
  index_block = 'I',
  meta_block = 'M',
  unknown_block = 'U',
};

static const std::map<char, std::string> BlockTypeMap{
    {static_cast<char>(BlockType::data_block), "Data"},
    {static_cast<char>(BlockType::entity_block), "Entity"},
    {static_cast<char>(BlockType::free_block), "Free"},
    {static_cast<char>(BlockType::index_block), "Index"},
    {static_cast<char>(BlockType::meta_block), "Meta"},
    {static_cast<char>(BlockType::unknown_block), "Unknow"},
};

// a small header that describes the block...
constexpr const size_t kExtraSize = 16;
struct BlockHeader {
  explicit BlockHeader(BlockType aType = BlockType::data_block);

  void setType(BlockType aType);
  void setToMeta();
  void setToLookup();
  void setToIndex(uint32_t aBlockNum, const std::string &anExtra);
  void setToFree();
  void setExtra(const std::string &anExtra);

  uint32_t count{0};  // how many blocks belong to this
  uint32_t pos{0};    // block number  //? pos might not indicate blockNum
  uint32_t next{0};   // next block number to read if data cannot fit 1kB
  uint32_t entityHash{0};
  float version{0.0};
  char type{'U'};  // char version of block type
  std::array<char, kExtraSize> extra{0};
};

constexpr const size_t kBlockSize = 1024;
constexpr const size_t kPayloadSize = kBlockSize - sizeof(BlockHeader);

// block .................
struct Block {
  explicit Block(BlockType aType = BlockType::data_block);

  Block &setMetaHeader();
  Block &setLookupHeader();
  Block &setAsFree();
  [[nodiscard]] bool isTypeMatch(const BlockType &aBlockType) const;
  [[nodiscard]] bool isIdMatch(uint32_t aHash) const;

  std::array<char, kPayloadSize> payload{0};  // actual data
  BlockHeader header;                         // header data
  friend std::ostream &operator<<(std::ostream &aStream, const Block &aBlock);
};

//------------------------------
class BlockIO {
 public:
  BlockIO(const std::string &aPath, const std::ios_base::openmode &aMode);

  virtual ~BlockIO();
  virtual uint32_t getBlockCount();
  virtual StatusResult readBlock(uint32_t aBlockNum, Block &aBlock);
  virtual StatusResult writeBlock(uint32_t aBlockNum, Block &aBlock, bool aChange=true);
  StatusResult createAndSaveSpecialBlock(uint32_t aBlockNum, BlockType aType,
                                         const std::string &anExtra = "");

 protected:
  std::fstream stream;
  LRUCache<uint32_t, Block> blockCache;
};

bool isMatchedMetaBlock(const Block &aBlock, uint32_t aHash);
bool isMatchedEntityBlock(const Block &aBlock, uint32_t aHash);
bool isMatchedDataBlock(const Block &aBlock, uint32_t aHash);
}  // namespace ECE141

#endif /* BlockIO_hpp */
