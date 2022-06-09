/**
 * @file BlockIO.cpp
 * @author Yifan Wu, Samuel Woo
 * @brief
 * @version 0.5
 * @date 2022-04-18
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "BlockIO.hpp"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

#include "Config.hpp"
#include "Errors.hpp"
#include "Helpers.hpp"

namespace ECE141 {

//---------------------------------------------------
// BlockHeader interface
BlockHeader::BlockHeader(BlockType aType)
    : version{std::stof(Config::getVersion())},
      type{static_cast<char>(aType)} {}

void BlockHeader::setType(BlockType aType) { type = static_cast<char>(aType); }

void BlockHeader::setToMeta() {
  setType(BlockType::meta_block);
  count = 1;
  pos = META_BLOCK_NUM;  // reserve block 0 for Meta block in each DB
  next = 0;
  entityHash = Helpers::hashString(kMetaBlockHash);
}

void BlockHeader::setToLookup() {
  setType(BlockType::meta_block);
  count = 1;
  pos = LOOKUP_BLOCK_NUM;  // reserve block 1 for Index block in each DB
  next = 0;
  entityHash = Helpers::hashString(kLookupBlockHash);
}

void BlockHeader::setToIndex(uint32_t aBlockNum, const std::string &anExtra) {
  setType(BlockType::index_block);
  count = 1;
  pos = aBlockNum;
  next = 0;
  setExtra(anExtra);
  // entityHash
}

void BlockHeader::setToFree() {
  setType(BlockType::free_block);
  count = 0;
  // pos = 0;
  next = 0;
  entityHash = 0;
}

void BlockHeader::setExtra(const std::string &anExtra) {
  for (size_t i = 0; i < anExtra.size(); i++) {
    extra.at(i) = anExtra.at(i);
  }
}

//---------------------------------------------------
// Block interface
Block::Block(BlockType aType) : header{aType} {}

Block &Block::setMetaHeader() {
  header.setToMeta();
  header.setExtra("Meta");
  return *this;
}

Block &Block::setLookupHeader() {
  header.setToLookup();
  header.setExtra("LookUp");
  return *this;
}

Block &Block::setAsFree() {
  header.setToFree();
  payload.fill('\0');
  return *this;
}

auto Block::isTypeMatch(const BlockType &aBlockType) const -> bool {
  return static_cast<char>(aBlockType) == header.type;
}

auto Block::isIdMatch(uint32_t aHash) const -> bool {
  return aHash == header.entityHash;
}

std::ostream &operator<<(std::ostream &aStream, const Block &aBlock) {
  std::string extra;
  std::copy(aBlock.header.extra.begin(), aBlock.header.extra.end(),
            std::back_inserter(extra));
  aStream << "+-------------------------------------------------------------\n";
  aStream << "| HEADER\n";
  aStream << "+--------------\n"
          << "| Type: " << BlockTypeMap.at(aBlock.header.type) << '\n'
          << "| Count: " << aBlock.header.count << '\n'
          << "| Pos: " << aBlock.header.pos << '\n'
          << "| Next: " << aBlock.header.next << '\n'
          << "| EntityHash: " << aBlock.header.entityHash << '\n'
          << "| Version: " << aBlock.header.version << '\n'
          << "| Extra: " << extra << '\n'
          << "+--------------\n";
  aStream << "| PAYLOAD:\n| #";
  for (const char &c : aBlock.payload) {
    aStream << c;
  }
  aStream << "|end|\n";
  aStream << "+-------------------------------------------------------------\n";
  return aStream;
}

//---------------------------------------------------
// BlockIO interface
BlockIO::BlockIO(const std::string &aPath, const std::ios_base::openmode &aMode)
    : stream{aPath, aMode} {
  if (!stream.is_open()) {
    throw std::runtime_error("File opening failed!");
  }
  if(Config::useCache(CacheType::block)) {
      blockCache.setMaxsize(Config::getCacheSize(CacheType::block));
  }
}

BlockIO::~BlockIO() {
  stream.close();  // ensure we close the stream after destruction
}

// ---------------------------------------
// USE: read data from stream (a DB File) & write into a given block (after
// seek)
auto BlockIO::readBlock(uint32_t aBlockNum, Block &aBlock) -> StatusResult {
  if(Config::useCache(CacheType::block)) {
    if(blockCache.contains(aBlockNum)) {
        aBlock = blockCache.get(aBlockNum);
        return {Errors::noError};
    }
  }
  if (stream) {
    stream.seekg(aBlockNum * kBlockSize, std::ios::beg);
    stream.read(reinterpret_cast<char *>(&aBlock), kBlockSize);
//    if(Config::useCache(CacheType::block)) {
//        blockCache.put(aBlockNum, aBlock);
//    }
    return {Errors::noError};
  }
  return {Errors::readError};
}

// USE: write data into stream (a DB File) by reading from a given block (after
// seek)
auto BlockIO::writeBlock(uint32_t aBlockNum, Block &aBlock, bool aChange) -> StatusResult {
  if (stream) {
    stream.seekg(aBlockNum * kBlockSize, std::ios::beg);
    stream.write(reinterpret_cast<char *>(&aBlock), kBlockSize);
    if(Config::useCache(CacheType::block) && aChange) {
        blockCache.deleteKey(aBlockNum); //delete existing block from cache so it can be updated
        blockCache.put(aBlockNum, aBlock);
    }
    return {Errors::noError};
  }
  return {Errors::writeError};
}

StatusResult BlockIO::createAndSaveSpecialBlock(uint32_t aBlockNum,
                                                BlockType aType,
                                                const std::string &anExtra) {
  Block theBlock{aType};
  if (BlockType::meta_block == aType) {
    if (LOOKUP_BLOCK_NUM == aBlockNum) {
      theBlock.setLookupHeader();
    } else {
      theBlock.setMetaHeader();
    }
  } else if (BlockType::index_block == aType) {
    theBlock.header.setToIndex(aBlockNum, anExtra);

  } else {
    std::cerr << "Only implement Meta and Index\n";
    return {Errors::notImplemented};
  }
  writeBlock(aBlockNum, theBlock);
  // std::cerr << theBlock << '\n';  // uncomment for debug
  return {Errors::noError};
}

// ---------------------------------------
// USE: count blocks in file ---------------------------------------
auto BlockIO::getBlockCount() -> uint32_t {
  if (stream) {
    stream.seekg(0, std::ios::end);
    const auto theEnd = stream.tellg();
    stream.seekg(0, std::ios::beg);
    const auto theBeg = stream.tellg();
    size_t length = theEnd - theBeg;
    return static_cast<uint32_t>(length / kBlockSize);
  }
  std::cerr << std::boolalpha << "stream is_open(): " << stream.is_open()
            << '\n';
  // means stream error
  return 0;
}

auto isMatchedMetaBlock(const Block &aBlock, uint32_t aHash) -> bool {
  return aBlock.isIdMatch(aHash) && aBlock.isTypeMatch(BlockType::meta_block);
}

auto isMatchedEntityBlock(const Block &aBlock, uint32_t aHash) -> bool {
  return aBlock.isIdMatch(aHash) && aBlock.isTypeMatch(BlockType::entity_block);
}

auto isMatchedDataBlock(const Block &aBlock, uint32_t aHash) -> bool {
  return aBlock.isIdMatch(aHash) && aBlock.isTypeMatch(BlockType::data_block);
}

}  // namespace ECE141
