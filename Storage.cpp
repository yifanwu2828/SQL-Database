/**
 * @file Storage.cpp
 * @author Yifan Wu
 * @brief 
 * @version 0.9
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "Storage.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>  // for uint32_t, int32_t
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#include "BlockIO.hpp"
#include "Entity.hpp"
#include "Errors.hpp"
#include "Helpers.hpp"
#include "Index.hpp"
#include "Row.hpp"

namespace ECE141 {
StorageInfo::StorageInfo(size_t aRefId, size_t aSize, int32_t aStartPos,
                         BlockType aType, std::string anExtra)
    : extra{std::move(anExtra)},
      refId{aRefId},
      size{aSize},
      start{aStartPos},
      type{aType} {}

StorageInfo::StorageInfo(size_t aRefId, std::streampos aSize, int32_t aStartPos,
                         BlockType aType, std::string anExtra)
    : extra{std::move(anExtra)},
      refId{aRefId},
      size{static_cast<size_t>(aSize)},
      start{aStartPos},
      type{aType} {}

StorageInfo::StorageInfo(const BlockHeader &aHeader) { *this = aHeader; }

StorageInfo &StorageInfo::operator=(const BlockHeader &aHeader) {
  refId = aHeader.entityHash;
  size = aHeader.count;
  start = static_cast<int32_t>(aHeader.pos);
  type = static_cast<BlockType>(aHeader.type);
  std::copy(aHeader.extra.begin(), aHeader.extra.end(),
            std::back_inserter(extra));
  return *this;
}

std::ostream &operator<<(std::ostream &aStream, const StorageInfo &anInfo) {
  aStream << "+--------------------+\n";
  aStream << "| StorageInfo\n";
  aStream << "+--------------\n"
          << "| extra: " << anInfo.extra << '\n'
          << "| refId: " << anInfo.refId << '\n'
          << "| size: " << anInfo.size << '\n'
          << "| blockNum: " << anInfo.start << '\n'
          << "| BlockType: " << static_cast<char>(anInfo.type) << '\n';
  aStream << "+-----------------+\n";
  return aStream;
}

// Storage interface
Storage::Storage(const std::string &aPath, const std::ios_base::openmode &aMode)
    : BlockIO{aPath, aMode} {}

// USE: visitor  ---------------------------------------
bool Storage::each(BlockVisitor aVisitor) {
  size_t theCount = getBlockCount();
  Block theBlock;
  for (uint32_t i{0}; i < theCount; i++) {
    if (readBlock(i, theBlock)) {
      if (!aVisitor(theBlock, i)) {
        break;
      }
    }
  }
  return true;
}

// USE: pos of next free (or new)...---------------------------------------
auto Storage::getFreeBlock() -> uint32_t {
  if (!available.empty()) {
    uint32_t theFreeBlkNum = available.front();
    available.pop_front();
    return theFreeBlkNum;
  }
  return getBlockCount();
}

StatusResult Storage::markBlockAsFree(uint32_t aPos) {
  Block theBlock;
  theBlock.setAsFree();
  theBlock.header.pos = aPos;
  available.push_front(aPos);
  blockCache.deleteKey(aPos);
  return writeBlock(aPos, theBlock, false); //pass false because cache doesn't need to update deleted blocks
}

// USE: for use with storable API...
auto Storage::releaseBlocks(uint32_t aPos, bool aInclusive) -> StatusResult {
  Block theBlock;
  StatusResult theResult{Errors::noError};
  uint32_t theCount{0};  // Count how many blocks are removed
  while ((theResult = readBlock(aPos, theBlock))) {
    if (aInclusive) {
      markBlockAsFree(aPos);
      theCount++;
    }
    aPos = theBlock.header.next;
    aInclusive = true;
    if (0 == aPos) {
      break;
    }
  }
  theResult.value = theCount;
  return theResult;
}

StatusResult Storage::save(std::iostream &aStream, StorageInfo &anInfo,
                           [[maybe_unused]] const std::string &aDBName) {
  uint32_t theStartBlock = anInfo.start;
  if (anInfo.start != kNewBlock) {
    releaseBlocks(anInfo.start, false);  // free prior chain...
  } else {
    theStartBlock = getFreeBlock();
  }

  uint32_t theBlockNum = theStartBlock;
  StatusResult theResult{Errors::noError};
  // ? how many data associate with this block
  size_t theCount = ceil((anInfo.size / kPayloadSize) + 0.5);

  Block theBlock;  // Block's payload initialize to 0 by default

  while (theResult && (anInfo.size > 0)) {
    size_t theBufSize = std::min(anInfo.size, kPayloadSize);
    // extracts characters from stream to block
    aStream.read(reinterpret_cast<char *>(&theBlock.payload), theBufSize);
    anInfo.size -= theBufSize;

    theBlock.header.count = theCount;
    theBlock.header.pos = theBlockNum;
    theBlock.header.entityHash = anInfo.refId;
    theBlock.header.type = static_cast<char>(anInfo.type);

    for (size_t i = 0; i < anInfo.extra.size(); i++) {
      theBlock.header.extra.at(i) = anInfo.extra.at(i);
    }
    // if one block's capacity cannot contain the data
    theBlock.header.next = (anInfo.size > 0) ? getFreeBlock() : 0;
    // ? uncomment for debug
    // std::cerr << "Save to " << aDBName << " :\n" << theBlock;
    theResult = writeBlock(theBlockNum, theBlock);
  }
  theResult.value = theStartBlock;
  return theResult;
}

StatusResult Storage::load(std::iostream &anOut, StorageInfo &anInfo,
                           uint32_t aStartBlockNum,
                           [[maybe_unused]] const std::string &aDBName) {
  StatusResult theResult{Errors::seekError};
  if (getBlockCount() > aStartBlockNum) {
    Block theLoadBlock;
    theResult.error = Errors::noError;
    while (theResult) {
      if ((theResult = readBlock(aStartBlockNum, theLoadBlock))) {
        anInfo = theLoadBlock.header;
        // invoke copy assign of storage info from header
        // ? uncomment for debug
        // std::cerr << "In Load " << aDBName << ":\n" << theLoadBlock;
        anOut.write(reinterpret_cast<char *>(&theLoadBlock.payload),
                    kPayloadSize);
        aStartBlockNum = theLoadBlock.header.next;
        if (0 == aStartBlockNum) {
          // 0 is reserved for meta block
          break;
        }
      }
    }
  }
  return theResult;
}

// -----------------------------------------------------------------------------------
// IndexMap
StatusResult Storage::encodeIndexMap(std::ostream &anOutput,
                                     const IndexMap &anIndexMap) {
  for (const auto &[theTableName, theIndex] : anIndexMap) {
    Helpers::encodeInto(anOutput, theTableName);
    Helpers::encodeInto(anOutput, theIndex->getBlockNum());
  }
  return {Errors::noError};
}

StatusResult Storage::decodeIndexMap(std::istream &anInput,
                                     IndexMap &anIndexMap) {
  anIndexMap.clear();
  while (!(anInput >> std::ws).eof() && '\0' != anInput.peek()) {
    std::string theTableName;
    Helpers::decodeFrom(anInput, theTableName);
    uint32_t theIndexBlockNum;
    Helpers::decodeFrom(anInput, theIndexBlockNum);
    anIndexMap.emplace(theTableName,
                       std::make_unique<Index>(*this, theIndexBlockNum));
  }
  return {Errors::noError};
}

StatusResult Storage::saveIndexMap(const IndexMap &anIndexMap) {
  std::stringstream theMapStrm;
  auto theResult = encodeIndexMap(theMapStrm, anIndexMap);
  if (!(theMapStrm >> std::ws).eof() && theResult) {
    // collect storage info
    theMapStrm.seekg(0, std::ios::end);
    StorageInfo theInfo = getLookUpStorageInfo(theMapStrm.tellg());
    theMapStrm.seekg(0, std::ios::beg);
    theResult = save(theMapStrm, theInfo);
    // Encode each Index in map
    for (const auto &[theTableName, theIndex] : anIndexMap) {
      // TODO: no need to overwrite if index not change.(bug)
      // if(!theIndex->isChanged()) {
      //   continue;
      // }
      std::stringstream theIndexStrm;
      theResult = theIndex->encode(theIndexStrm);
      if (!(theIndexStrm >> std::ws).eof() && theResult) {
        theIndexStrm.seekg(0, std::ios::end);
        StorageInfo theIndexInfo =
            theIndex->getStorageInfo(theIndexStrm.tellg(), theTableName);
        theIndexStrm.seekg(0, std::ios::beg);
        theResult = save(theIndexStrm, theIndexInfo);
      }
    }
  }
  return theResult;
}

StatusResult Storage::loadIndexMap(IndexMap &anIndexMap) {
  std::stringstream theMapStrm;
  StorageInfo theIdxMapInfo;
  auto theResult = load(theMapStrm, theIdxMapInfo, LOOKUP_BLOCK_NUM);
  // std::cerr << "#|" << theMapStrm.str() << "|\n";
  if (!(theMapStrm >> std::ws).eof() && theResult &&
      '\0' != theMapStrm.peek()) {
    theResult = decodeIndexMap(theMapStrm, anIndexMap);

    // Need to decode each index in IndexMap
    for (auto &[theTableName, theIndex] : anIndexMap) {
      std::stringstream theIndexStrm;
      StorageInfo theIndexInfo;
      theResult = load(theIndexStrm, theIndexInfo, theIndex->getBlockNum());
      // std::cerr << "#|" << theIndexStrm.str() << "|\n";
      if (!(theIndexStrm >> std::ws).eof() && theResult) {
        theResult = theIndex->decode(theIndexStrm);
      }
    }
  }
  return theResult;
}

StatusResult Storage::getRowsByIndex(const std::string &anEntityName,
                                     const IndexMap &anIndexMap,
                                     RowCollection &aCollection) {
  StatusResult theResult{Errors::entityBlockNumNotFound};
  if (anIndexMap.find(anEntityName) != anIndexMap.end()) {
    auto &theIndex = anIndexMap.at(anEntityName);
    auto theBlkVisitor = [&]([[maybe_unused]] const Block &aBlock,
                             uint32_t aBlockNum) {
      // working directly with Index to get Row
      std::stringstream theDecodeStream;
      StorageInfo theLoadInfo;
      if (!(theResult = load(theDecodeStream, theLoadInfo, aBlockNum))) {
        std::cerr << "Index storage.load() fail\n";
        return false;
      }
      auto theMatchedRow =
          std::make_unique<Row>(theLoadInfo.refId, theLoadInfo.start);
      if (!(theResult = theMatchedRow->decode(theDecodeStream))) {
        std::cerr << "Index row decode fail\n";
        return false;
      }
      // std::cout << *theMatchedRow;
      aCollection.push_back(std::move(theMatchedRow));
      return true;
    };
    theIndex->each(theBlkVisitor);
  }
  return theResult;
}

StatusResult Storage::getRowsByBruteForce(const Entity &anEntity,
                                          RowCollection &aCollection) {
  return getRowsByBruteForce(anEntity.getName(), aCollection);
}

StatusResult Storage::getRowsByBruteForce(const std::string &anEntityName,
                                          RowCollection &aCollection) {
  StatusResult theResult{Errors::readError};
  uint32_t theHash = Helpers::hashString(anEntityName);
  // working directly with storage to get rows
  auto theBlkVisitor = [&](const Block &aBlock, uint32_t aBlockNum) {
    if (aBlock.isIdMatch(theHash) &&
        aBlock.isTypeMatch(BlockType::data_block)) {
      std::stringstream theDecodeStream;
      StorageInfo theLoadInfo;
      theResult = load(theDecodeStream, theLoadInfo, aBlockNum);
      if (!theResult) {
        std::cerr << "Itr storage.load() fail\n";
        return false;
      }
      std::unique_ptr<Row> theMatchedRow =
          std::make_unique<Row>(theLoadInfo.refId, theLoadInfo.start);
      if (!theMatchedRow->decode(theDecodeStream)) {
        std::cerr << "Itr row decode fail\n";
        return false;
      }
      // std::cout << *theMatchedRow;
      aCollection.push_back(std::move(theMatchedRow));
    }
    return true;
  };
  each(theBlkVisitor);
  return theResult;
}

StatusResult Storage::dropRowsByBruteForce(const std::string &anEntityName) {
  StatusResult theResult{Errors::writeError};
  size_t theCount{0};
  uint32_t theHash = Helpers::hashString(anEntityName);
  // working directly with storage to get rows
  auto theBlkVisitor = [&](const Block &aBlock, uint32_t aBlockNum) {
    // this only check a data Block with same hash,
    if (isMatchedDataBlock(aBlock, theHash)) {
      theResult = releaseBlocks(aBlockNum, true);
      if (!theResult) {
        std::cerr << "Data Block Drop Fail\n";
        return false;
      }
      theCount++;
    }
    return true;
  };
  if (each(theBlkVisitor)) {
    theResult.value = theCount;
  }
  return theResult;
}

StatusResult Storage::dropRowsByIndex(const std::string &anEntityName,
                                      IndexMap &anIndexMap) {
  StatusResult theResult{Errors::entityBlockNumNotFound};
  if (anIndexMap.find(anEntityName) != anIndexMap.end()) {
    auto &theIndex = anIndexMap.at(anEntityName);
    // working directly with Index to get BlockNum
    uint32_t theCount{0};
    auto theBlkVisitor = [&]([[maybe_unused]] const Block &aBlock,
                             uint32_t aBlockNum) {
      theResult = releaseBlocks(aBlockNum, true);
      if (!theResult) {
        std::cerr << "Indexed Data Block Drop Fail\n";
        return false;
      }
      theCount++;
      return true;
    };
    if (theIndex->each(theBlkVisitor)) {
      theResult.value = theCount;
    }
  }
  // Do not perform Index erase here
  return theResult;
}

// ---------------------------------------------------------------------------------
StatusResult Storage::saveMetaBlock(const Index &anIndex) {
  std::stringstream theEncodeStream;
  auto theResult = anIndex.encode(theEncodeStream);
  if (!theEncodeStream.eof() && theResult) {
    // collect storage info
    theEncodeStream.seekg(0, std::ios::end);
    StorageInfo theInfo = getMetaStorageInfo(theEncodeStream.tellg());
    theEncodeStream.seekg(0, std::ios::beg);

    theResult = save(theEncodeStream, theInfo);
  }
  return theResult;
}

StatusResult Storage::loadMetaBlock(Index &anIndex) {
  std::stringstream theDecodeStream;
  StorageInfo theLoadInfo;
  auto theResult = load(theDecodeStream, theLoadInfo, META_BLOCK_NUM);
  if (!(theDecodeStream >> std::ws).eof() && theResult) {
    theResult = anIndex.decode(theDecodeStream);
  }
  return theResult;
}

StorageInfo getMetaStorageInfo(std::streampos aSize) {
  return {Helpers::hashString(kMetaBlockHash), static_cast<size_t>(aSize),
          META_BLOCK_NUM, BlockType::meta_block, "Meta"};
}

StorageInfo getLookUpStorageInfo(std::streampos aSize) {
  return {Helpers::hashString(kLookupBlockHash), static_cast<size_t>(aSize),
          LOOKUP_BLOCK_NUM, BlockType::meta_block, "LookUp"};
}

StatusResult Storage::saveEntityBlock(const Entity &anEntity,
                                      int32_t BlockNum) {
  std::stringstream theEntityStrm;
  const std::string &theEntityName = anEntity.getName();
  anEntity.encode(theEntityStrm);
  if (!(theEntityStrm >> std::ws).eof()) {
    theEntityStrm.seekg(0, std::ios::end);
    StorageInfo theEntityInfo{Helpers::hashString(theEntityName),
                              theEntityStrm.tellg(), BlockNum,
                              BlockType::entity_block, theEntityName};
    theEntityStrm.seekg(0, std::ios::beg);
    return save(theEntityStrm, theEntityInfo);
  }
  return {Errors::noEncodePerform};
}

// Update Row should save dataBlock again
StatusResult Storage::saveDataBlock(const Row &aRow, int32_t BlockNum) {
  std::stringstream theRowStrm;
  aRow.encode(theRowStrm);
  if (!(theRowStrm >> std::ws).eof()) {
    theRowStrm.seekg(0, std::ios::end);
    StorageInfo theRowEncodeInfo{aRow.getEntityId(), theRowStrm.tellg(),
                                 BlockNum, BlockType::data_block};
    theRowStrm.seekg(0, std::ios::beg);
    return save(theRowStrm, theRowEncodeInfo);
  }
  return {Errors::noEncodePerform};
}

}  // namespace ECE141
