/**
 * @file Database.cpp
 * @author Yifan Wu
 * @brief
 * @version 0.9
 * @date 2022-05-30
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "Database.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "Attribute.hpp"
#include "BasicTypes.hpp"
#include "BlockIO.hpp"
#include "Config.hpp"
#include "Entity.hpp"
#include "Errors.hpp"
#include "Filters.hpp"
#include "Query.hpp"
#include "Storage.hpp"
#include "TableFormatter.hpp"
#include "keywords.hpp"

namespace ECE141 {

constexpr const auto CreateNew = std::ios::in | std::ios::out | std::ios::trunc;
constexpr const auto OpenExisting = std::ios::in | std::ios::out;

Database::Database(const std::string &aName, CreateDB)
    : name{aName},
      changed{true},
      storage{Config::getDBPath(aName), CreateNew},
      entityIndex{storage, META_BLOCK_NUM, IndexType::strKey},
      debugInfo{"CreateDB"} {
  // write block to db file
  storage.createAndSaveSpecialBlock(META_BLOCK_NUM, BlockType::meta_block);
  storage.createAndSaveSpecialBlock(LOOKUP_BLOCK_NUM, BlockType::meta_block);
  // at this time palyLoad in both metaBlock and indexBlock are empty(initialize
  // with zeros)
}
Database::Database(const std::string &aName, OpenDB)
    : name{aName},
      changed{false},
      storage{Config::getDBPath(aName), OpenExisting},
      entityIndex{storage, META_BLOCK_NUM, IndexType::strKey},
      debugInfo{"OpenDB"} {
  if (!(storage.loadMetaBlock(entityIndex))) {
    throw std::runtime_error("Fail to load Meta Block!");
  }
  if (!(storage.loadIndexMap(indexMap))) {
    throw std::runtime_error("Fail to load LookUp Block!");
  }
}

Database::~Database() {
  // std::cerr << std::boolalpha << '\n'
  //           << getName() << "::~Database() change: " << changed << '\n';
  if (changed) {
    storage.saveMetaBlock(entityIndex);
  }
  storage.saveIndexMap(indexMap);  // TODO: when should save?
  // ~BlockIO() ensure we close the stream after destruction
}

// ----------------------------------------------
// setters
void Database::setChanged(bool aChange) { changed |= aChange; }
void Database::setDebugInfo(std::string &&anInfo) {
  debugInfo = std::move(anInfo);
}

// getters
std::string Database::getName() const { return name; }
Storage &Database::getStorage() { return storage; }
Index &Database::getEntityIndex() { return entityIndex; }
IndexMap &Database::getIndexMap() { return indexMap; }
// ----------------------------------------------
// TODO: TabularView
// USE: Call this to dump the db for debug purposes...
auto Database::dump(std::ostream &anOutput) -> StatusResult {
  anOutput.setf(std::ios::left, std::ios::adjustfield);
  constexpr std::streamsize theWidth{11};
  constexpr size_t theColNum{8};
  std::vector<std::streamsize> widths(theColNum, theWidth + 1);
  TableFormatter::printBreak(anOutput, widths);
  std::vector<std::string> theTitles{" BlockNum", " Type", " Hash ID",
                                     " Version",  " Next", " Count",
                                     " Extra"};
  anOutput.fill(' ');
  anOutput.width(theWidth);
  anOutput << "| Idx"
           << " |";
  for (const auto &title : theTitles) {
    anOutput.fill(' ');
    anOutput.width(theWidth);
    anOutput << title << "|";
  }
  anOutput << "\n";
  TableFormatter::printBreak(anOutput, widths);

  size_t theIndex = 0;
  auto theOutputVisitor = [&](const Block &aBlock,
                              [[maybe_unused]] uint32_t aBlockNum) {
    anOutput.fill(' ');
    // true Idx
    anOutput.width(theWidth);
    anOutput << "| " + std::to_string(theIndex++) << " |";

    // TODO:
    // // Block Num
    // anOutput.width(theWidth);
    // anOutput << "| " + std::to_string(aBlock.header.pos) << " |";

    // BlockNum
    anOutput.width(theWidth);
    anOutput << aBlock.header.pos << "|";
    // Type
    anOutput.width(theWidth);
    anOutput << BlockTypeMap.at(aBlock.header.type) << "|";
    // ID
    anOutput.width(theWidth);
    anOutput << aBlock.header.entityHash << "|";
    // Version
    anOutput.width(theWidth);
    anOutput << std::setprecision(1) << aBlock.header.version << "|";
    // Next
    anOutput.width(theWidth);
    anOutput << aBlock.header.next << "|";
    // Count
    anOutput.width(theWidth);
    anOutput << aBlock.header.count << "|";
    // Extra
    std::string theExtra;
    std::copy(aBlock.header.extra.begin(), aBlock.header.extra.end(),
              std::back_inserter(theExtra));
    anOutput.width(theWidth);
    anOutput << theExtra << "|\n";
    TableFormatter::printBreak(anOutput, widths);
    return true;
  };
  storage.each(theOutputVisitor);
  TableFormatter::printRowsInSet(anOutput, storage.getBlockCount());
  return {Errors::noError};
}

auto Database::entityExistsInDB(const std::string &aName) const -> bool {
  return entityIndex.exists(aName);
}

// (Order by) if limit N, only sort first N element.
void sortRows(RowCollection &aCollection, const StringList &anOrderList,
              size_t aLimit) {
  if (!anOrderList.empty() && !aCollection.empty()) {
    auto cmp = [&](const auto &a, const auto &b) {
      for (const std::string &theOrder : anOrderList) {
        if (a->getData().at(theOrder) == b->getData().at(theOrder)) {
          continue;
        }
        return a->getData().at(theOrder) < b->getData().at(theOrder);
      }
      return true;
    };
    // if limit N, only sort first N element.
    if (aLimit > 0) {
      std::partial_sort(aCollection.begin(),
                        aCollection.begin() + static_cast<long>(aLimit),
                        aCollection.end(), cmp);
    } else {
      std::sort(aCollection.begin(), aCollection.end(), cmp);
    }
  }
}

StatusResult Database::getAllRowsFrom(const std::string &aTableName,
                                      RowCollection &aCollection) {
  StatusResult theResult{Errors::unknownTable};
  if (entityExistsInDB(aTableName)) {
    // get all rows associate with theTableName
    if (Config::useIndex()) {
      theResult = storage.getRowsByIndex(aTableName, indexMap, aCollection);
    } else {
      theResult = storage.getRowsByBruteForce(aTableName, aCollection);
    }
  }
  return theResult;
}

using JoinFactory =
    std::function<StatusResult(const Join &, const RowCollection &,
                               const RowCollection &, RowCollection &)>;

StatusResult Database::joinRows(const JoinList &aJoinList,
                                RowCollection &aCollection) {
  StatusResult theResult{};
  RowCollection theJoinResult;

  if (!aJoinList.empty()) {
    for (const auto &theJoin : aJoinList) {
      RowCollection theJoinRows;
      theResult = getAllRowsFrom(theJoin.table, theJoinRows);
      if (!theResult) {
        return theResult;
      }
      static std::map<Keywords, JoinFactory> factories{
          {Keywords::left_kw, leftJoinRows},
          {Keywords::right_kw, rightJoinRows},
          {Keywords::inner_kw, innerJoinRows},
          {Keywords::cross_kw, crossJoinRows},
          {Keywords::full_kw, fullJoinRows},
      };
      if (factories.find(theJoin.joinType) != factories.end()) {
        theResult = factories[theJoin.joinType](theJoin, aCollection,
                                                theJoinRows, theJoinResult);
        if (!theResult) {
          return theResult;
        }
      } else {
        return {Errors::unexpectedKeyword};
      }
    }
    std::swap(aCollection, theJoinResult);
  }
  return theResult;
}

StatusResult Database::selectRow(const DBQuery &aQuery,
                                 RowCollection &aCollection) {
  auto theResult = getAllRowsFrom(aQuery.getEntityName(), aCollection);
  if (!theResult) {
    return theResult;
  }

  const JoinList &theJoinList = aQuery.getJoins();
  if (!theJoinList.empty()) {
    theResult = joinRows(theJoinList, aCollection);
    if (!theResult) {
      return theResult;
    }
  }

  StringList theOrderList{aQuery.getOrderBy()};
  size_t theLimit = aQuery.getLimit();
  // where
  if (aQuery.getExpressionNum() > 0) {
    aCollection.erase(
        std::remove_if(
            aCollection.begin(), aCollection.end(),
            [&](auto &x) { return !aQuery.getFilter().matches(x->getData()); }),
        aCollection.end());
  }
  // Order by
  if (!theOrderList.empty()) {
    sortRows(aCollection, theOrderList, theLimit);
  }
  // Limit
  if (theLimit > 0 && aCollection.size() > theLimit) {
    aCollection.erase(aCollection.begin() + static_cast<long>(theLimit),
                      aCollection.end());
  }
  return {Errors::noError};
}

auto Database::updateRow(const DBQuery &aQuery, RowCollection &aCollection)
    -> StatusResult {
  auto theResult = selectRow(aQuery, aCollection);
  // std::string theEntityName{aQuery.getEntityName()};
  for (auto &theRow : aCollection) {
    const auto &[theKey, theVal] = aQuery.getUpdateKV();
    theRow->getData()[theKey] = theVal;
    theResult = storage.saveDataBlock(
        *theRow, static_cast<int32_t>(theRow->getBlockNum()));
  }
  return theResult;
}

auto Database::deleteRow(const DBQuery &aQuery, RowCollection &aCollection)
    -> StatusResult {
  // delete the data block and remove it's associate index in indexMap
  auto theResult = selectRow(aQuery, aCollection);
  const Entity &theEntity = aQuery.getEntity();
  const Attribute *thePrimaryKey = theEntity.getPrimaryKey();
  auto &theIndex = indexMap[aQuery.getEntityName()];

  std::for_each(
      aCollection.begin(), aCollection.end(), [&](const auto &theRow) {
        Value theVal = theRow->getData().at(thePrimaryKey->getName());
        auto theIdxKey =
            Index::valueToIndexKey(theVal, thePrimaryKey->getType());
        if (IndexType::intKey == theIndex->getType()) {
          // std::cerr << std::get<uint32_t>(theIdxKey) << ", "
          // << theIndex->valueAt(theIdxKey).value() << '\n';
          theIndex->erase(std::get<uint32_t>(theIdxKey));
        } else {
          theIndex->erase(std::get<std::string>(theIdxKey));
        }
        theIndex->setChanged(true);
        theResult = storage.releaseBlocks(theRow->getBlockNum(), true);
      });
  return theResult;
}

auto leftJoinRows(const Join &aJoin, const RowCollection &aLHSCollection,
                  const RowCollection &aRHSCollection,
                  RowCollection &aJoinedCollection) -> StatusResult {
  if (!aJoin.exprs.empty() && !aRHSCollection.empty()) {
    Row theNullRow{*(aRHSCollection[0])};
    theNullRow.setAllNull();
    for (const auto &RowLHS : aLHSCollection) {
      bool matchAny = false;
      auto collect = [&](const auto &RowRHS) {
        if (Filters::matches(aJoin.exprs, RowLHS->getData(),
                             RowRHS->getData())) {
          aJoinedCollection.push_back(std::make_unique<Row>(*RowLHS + *RowRHS));
          matchAny = true;
        }
      };
      std::for_each(aRHSCollection.begin(), aRHSCollection.end(), collect);
      if (!matchAny) {
        aJoinedCollection.push_back(
            std::make_unique<Row>(*RowLHS + theNullRow));
      }
    }
  }
  return {Errors::noError};
}

auto rightJoinRows(const Join &aJoin, const RowCollection &aLHSCollection,
                   const RowCollection &aRHSCollection,
                   RowCollection &aJoinedCollection) -> StatusResult {
  return leftJoinRows(aJoin, aRHSCollection, aLHSCollection, aJoinedCollection);
}

auto innerJoinRows(const Join &aJoin, const RowCollection &aLHSCollection,
                   const RowCollection &aRHSCollection,
                   RowCollection &aJoinedCollection) -> StatusResult {
  return {Errors::notImplemented};
}

auto crossJoinRows(const Join &aJoin, const RowCollection &aLHSCollection,
                   const RowCollection &aRHSCollection,
                   RowCollection &aJoinedCollection) -> StatusResult {
  return {Errors::notImplemented};
}

auto fullJoinRows(const Join &aJoin, const RowCollection &aLHSCollection,
                  const RowCollection &aRHSCollection,
                  RowCollection &aJoinedCollection) -> StatusResult {
  return {Errors::notImplemented};
}
}  // namespace ECE141
