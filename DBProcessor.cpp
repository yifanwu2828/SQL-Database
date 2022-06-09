/**
 * @file DBProcessor.cpp
 * @author Yifan Wu
 * @brief 
 * @version 0.9
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "DBProcessor.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <utility>
#include <variant>
#include <vector>

#include "Application.hpp"
#include "Attribute.hpp"
#include "BlockIO.hpp"
#include "Config.hpp"
#include "DBStatement.hpp"
#include "Database.hpp"
#include "Entity.hpp"
#include "Errors.hpp"
#include "Index.hpp"
#include "Statement.hpp"
#include "Storage.hpp"
#include "TableFormatter.hpp"
#include "Timer.hpp"
#include "Tokenizer.hpp"
#include "keywords.hpp"

namespace ECE141 {

// like a typedef!!!
// using DBStmtFactory = Statement* (*)(Application* anApp);
using DBStmtFactory = std::function<Statement*(Application*)>;
// * Database StatementFactory
Statement* createDBStmtFactory(Application* anApp) {
  return new CreateDBStatement(anApp);
}

Statement* dropDBStmtFactory(Application* anApp) {
  return new DropDBStatement(anApp);
}

Statement* useDBsStmtFactory(Application* anApp) {
  return new UseDBStatement(anApp);
}

Statement* showDBStmtFactory(Application* anApp) {
  return new ShowDBStatement(anApp);
}

Statement* dumpDBStmtFactory(Application* anApp) {
  return new DumpDBStatement(anApp);
}

// ---------------------------------------------------------------
// DBProcessor class
DBProcessor::DBProcessor(std::ostream& anOutput, Application* anApp)
    : CmdProcessor{anOutput, "DBprocessor"}, app{anApp}, sql{anOutput, this} {}

bool DBProcessor::entityExistsInDB(const std::string& aName) const {
  if (auto* theActiveDB = getActiveDB()) {
    return theActiveDB->entityExistsInDB(aName);
  }
  return false;
}

// CmdProcessor interface ...
CmdProcessor* DBProcessor::recognizes(Tokenizer& aTokenizer) {
  if (CreateDBStatement::recognize(aTokenizer) ||
      DropDBStatement::recognize(aTokenizer) ||
      UseDBStatement::recognize(aTokenizer) ||
      ShowDBStatement::recognize(aTokenizer) ||
      DumpDBStatement::recognize(aTokenizer)) {
    return this;  // dbProcessor
  }
  return sql.recognizes(aTokenizer);  // will return sqlProcessor or nullptr
}

auto DBProcessor::run(Statement* aStmt) -> StatusResult {
  return aStmt->run(output);  // from cmdProcessor
}

// USE: retrieve a statement based on given text input...
Statement* DBProcessor::makeStatement(Tokenizer& aTokenizer,
                                      [[maybe_unused]] StatusResult& aResult) {
  static std::map<Keywords, DBStmtFactory> factories{
      {Keywords::create_kw, createDBStmtFactory},
      {Keywords::drop_kw, dropDBStmtFactory},
      {Keywords::dump_kw, dumpDBStmtFactory},
      {Keywords::show_kw, showDBStmtFactory},
      {Keywords::use_kw, useDBsStmtFactory},
  };

  const Keywords& theFirstKW = aTokenizer.current().keyword;
  if (factories.find(theFirstKW) != factories.end()) {
    if (Statement* theStatement = (factories.at(theFirstKW))(app)) {
      if (theStatement->parse(aTokenizer)) {
        return theStatement;
      }
    }
  }
  return nullptr;  // means we don't know the statement...
}

// ---------------------------------------------------------------
// sql interface
// When CREATE a table, automatically
// create an index associated for the primary key of the table.
// The value of every index is the block number of the row associated with the
// primary key.
auto DBProcessor::createTable(Entity* anEntity) -> StatusResult {
  StatusResult theResult{Errors::noDatabaseInUse};
  if (auto* theActiveDB = app->getDatabaseInUse()) {
    theActiveDB->setDebugInfo("createTable");
    Index& theEntityIndex = theActiveDB->getEntityIndex();
    IndexMap& theIndexMap = theActiveDB->getIndexMap();
    Storage& theStorage = theActiveDB->getStorage();
    std::string theDBName{theActiveDB->getName()};
    std::string theTableName{anEntity->getName()};

    // make sure table not already exist
    if (!theActiveDB->entityExistsInDB(theTableName)) {
      theActiveDB->setChanged(true);
      // Reserve an Index Block for this Entity
      const Attribute* thePrimaryKey = anEntity->getPrimaryKey();
      IndexType theIndexKeyType =
          (DataTypes::int_type == thePrimaryKey->getType()) ? IndexType::intKey
                                                            : IndexType::strKey;
      uint32_t theIndexBlockNum = theStorage.getFreeBlock();
      theStorage.createAndSaveSpecialBlock(
          theIndexBlockNum, BlockType::index_block,
          theTableName + '.' + thePrimaryKey->getName());

      // Add to IndexMap
      theIndexMap.emplace(
          theTableName,
          std::make_unique<Index>(theStorage, theIndexBlockNum, theIndexKeyType,
                                  thePrimaryKey->getName()));

      // collect storage info for entity block
      theResult = theStorage.saveEntityBlock(*anEntity);

      // theResult.value is the calculated BlockNum
      theEntityIndex.setKeyValue(theTableName, theResult.value);
      TableFormatter::printStatusRowDuration(output, theResult, 1,
                                             Config::getTimer().elapsed());
    } else {
      theResult.error = Errors::tableExists;
    }
  }
  return theResult;
}

auto DBProcessor::describeTable(const std::string& aName) -> StatusResult {
  StatusResult theResult{Errors::noDatabaseInUse};
  if (auto* theActiveDB = app->getDatabaseInUse()) {
    theActiveDB->setDebugInfo("describeTable");
    if (theActiveDB->entityExistsInDB(aName)) {
      Entity theEntity = createEntityFromStream(aName);

      std::vector<std::streamsize> theWidths{15, 10, 10, 10, 10, 30};
      output.setf(std::ios::left, std::ios::adjustfield);
      TableFormatter::printBreak(output, theWidths);
      output.width(theWidths[0] - 1);
      output << "|Field"
             << " |";
      output.width(theWidths[1] - 1);
      output << "Type"
             << "|";
      output.width(theWidths[2] - 1);
      output << "Null"
             << "|";
      output.width(theWidths[3] - 1);
      output << "Key"
             << "|";
      output.width(theWidths[4] - 1);
      output << "Default"
             << "|";
      output.width(theWidths[5] - 1);
      output << "Extra"
             << "|\n";
      TableFormatter::printBreak(output, theWidths);
      for (const auto& theAttr : theEntity.getAttributes()) {
        output.fill(' ');
        output.width(theWidths[0] - 1);
        output << "|" + theAttr.getName() << " |";
        output.width(theWidths[1] - 1);
        output << theAttr.getCharType() << "|";
        output.width(theWidths[2] - 1);
        std::string theNull;
        theAttr.isNullable() ? theNull = "YES" : theNull = "NO";
        output << theNull << "|";
        output.width(theWidths[3] - 1);
        std::string theKey;
        theAttr.isPrimaryKey() ? theKey = "YES" : theKey = "";
        output << theKey << "|";
        output.width(theWidths[4] - 1);
        std::string theDefault;
        (theAttr.getValue().index() != 0) ? theDefault = theAttr.getValueStr()
                                          : theDefault = "NULL";
        output << theDefault << "|";
        output.width(theWidths[5] - 1);
        std::string theExtra;
        if (theAttr.isAutoIncrement()) {
          theExtra += "auto_increment ";
        }
        if (theAttr.isPrimaryKey()) {
          theExtra += "primary key ";
        }
        output << theExtra << "|\n";
      }
      TableFormatter::printBreak(output, theWidths);
      TableFormatter::printRowsInSet(output, theEntity.getAttributes().size());
      TableFormatter::printDuration(output, Config::getTimer().elapsed());
      theResult.error = Errors::noError;
    } else {
      theResult.error = Errors::unknownTable;
    }
  }
  return theResult;
}

auto DBProcessor::dropTable(const std::string& aName) -> StatusResult {
  StatusResult theResult{Errors::noDatabaseInUse};
  if (auto* theActiveDB = app->getDatabaseInUse()) {
    theActiveDB->setDebugInfo("dropTable");
    Storage& theStorage = theActiveDB->getStorage();
    Index& theEntityIndex = theActiveDB->getEntityIndex();
    IndexMap& theIndexMap = theActiveDB->getIndexMap();

    theResult.error = Errors::unknownTable;
    if (theActiveDB->entityExistsInDB(aName)) {
      size_t theDropCount{0};
      // remove table from entityIndex and release the entity Block
      uint32_t theEntityBlkNum = theEntityIndex.valueAt(aName).value();
      theEntityIndex.erase(aName);
      if ((theResult = theStorage.releaseBlocks(theEntityBlkNum, true))) {
        theDropCount++;
      }

      // drop the data associate with tableName as well
      // do this before erase the map
      if (Config::useIndex()) {
        if ((theResult = theStorage.dropRowsByIndex(aName, theIndexMap))) {
          theDropCount += theResult.value;
        }
      } else {
        if ((theResult = theStorage.dropRowsByBruteForce(aName))) {
          theDropCount += theResult.value;
        }
      }

      // remove Index of the Table from Index map
      uint32_t theIndexBlkNum = theIndexMap.at(aName)->getBlockNum();
      theIndexMap.erase(aName);
      if ((theResult = theStorage.releaseBlocks(theIndexBlkNum, true))) {
        theDropCount++;
      }
      theActiveDB->setChanged(true);

      // -1 to not include index block
      TableFormatter::printStatusRowDuration(
          output, theResult, theDropCount - 1, Config::getTimer().elapsed());
    }
  }
  return theResult;
}

auto DBProcessor::showTables() -> StatusResult {
  StatusResult theResult{Errors::noDatabaseInUse};
  if (auto* theActiveDB = app->getDatabaseInUse()) {
    theActiveDB->setDebugInfo("showTables");
    std::vector<std::streamsize> theWidths = {28};

    output.setf(std::ios::left, std::ios::adjustfield);
    TableFormatter::printBreak(output, theWidths);
    output.fill(' ');
    output.width(theWidths[0]);
    output << "| Tables_in_" + theActiveDB->getName() << "|\n";
    TableFormatter::printBreak(output, theWidths);

    Index& theEntityIndex = theActiveDB->getEntityIndex();
    auto indexVisitor = [&](const IndexKey& theTableName,
                            [[maybe_unused]] uint32_t theBlockNum) -> bool {
      output.fill(' ');
      output.width(theWidths[0]);
      output << "| " + std::get<std::string>(theTableName) << "|\n";
      return true;
    };
    theEntityIndex.eachKV(indexVisitor);

    TableFormatter::printBreak(output, theWidths);
    TableFormatter::printRowsInSet(output, theEntityIndex.getSize());
    TableFormatter::printDuration(output, Config::getTimer().elapsed());
    theResult = Errors::noError;
  }
  return theResult;
}

auto DBProcessor::insertIntoTable(const std::string& aTableName,
                                  const RowCollection& aRowCollect)
    -> StatusResult {
  auto* theActiveDB = app->getDatabaseInUse();
  if (nullptr == theActiveDB) {
    return {Errors::noDatabaseInUse};
  }
  const std::string theDBName{theActiveDB->getName()};
  const Index& theEntityIndex = theActiveDB->getEntityIndex();
  IndexMap& theIndexMap = theActiveDB->getIndexMap();
  Storage& theStorage = theActiveDB->getStorage();

  StatusResult theResult{Errors::unknownTable};
  // make sure that entity/table name already exists in DB
  if (theActiveDB->entityExistsInDB(aTableName)) {
    // load Entity object from entity block
    Entity theEntity = createEntityFromStream(aTableName);

    // Get Primary Key for Index latter
    const Attribute* thePrimaryKey = theEntity.getPrimaryKey();
    std::string theKeyName = thePrimaryKey->getName();
    DataTypes theValType = thePrimaryKey->getType();

    // Use the loaded entity to verify the rows in the row collection
    std::vector<Attribute*> theAttributes;
    for (const auto& thePair : aRowCollect.front()->getData()) {
      // make sure attributes listed in insert exist in entity
      Attribute* theAttr = theEntity.getAttribute(thePair.first);
      if (theAttr != nullptr) {
        theAttributes.emplace_back(theAttr);
      }
    }

    theResult.error = Errors::invalidAttribute;
    if (theAttributes.size() == aRowCollect.front()->getData().size()) {
      // check all attributes are valid
      for (const auto& theRow : aRowCollect) {
        // time to validate the data types & nullability
        for (const auto& [theAttrName, theVal] : theRow->getData()) {
          Attribute* theAttr = theEntity.getAttribute(theAttrName);
          if (theRow->setValueForExistingKey(
                  theAttrName,
                  theAttr->toValue(std::get<std::string>(theVal)))) {
            // convert data to respective types
            theResult = theAttr->isInsertValid(theRow->getData());
            if (!theResult) {
              return theResult;
            }
          }
        }

        // insert autoincr IDs or default vals; if value not specified for
        // non-null attribute, return error
        for (const auto& theAttr : theEntity.getAttributes()) {
          if (theRow->getData().find(theAttr.getName()) ==
              theRow->getData().end()) {
            // check through all attributes in the entity that weren't listed
            // in the insert statement
            if (!theAttr.isNullable()) {
              if (theAttr.isAutoIncrement() && theAttr.isPrimaryKey()) {
                theRow->insert(theAttr.getName(), theEntity.getAutoIncrID());
              } else if (0 != theAttr.getValue().index()) {
                // if the attribute has a default value, add it to the row
                theRow->insert(theAttr.getName(), theAttr.getValue());
              } else {
                return theResult = Errors::nonNullableValueNotGiven;
              }
            }
          }
        }

        // create the data block and write to db-file
        theResult = theStorage.saveDataBlock(*theRow);

        // *this should modify the block num in sqlStatement rowCollect
        const uint32_t& theDataBlockNum = theResult.value;
        theRow->setBlockNum(theDataBlockNum);

        // add data BlockNum to IndexMap
        const KeyValues& theKV = theRow->getData();
        auto theIndexKey =
            Index::valueToIndexKey(theKV.at(theKeyName), theValType);
        theIndexMap[aTableName]->setKeyValue(theIndexKey, theDataBlockNum);
        // std::cerr << *theRow;
      }
      // Update autoinc to entity Block
      uint32_t theEntityBlockNum = theEntityIndex.valueAt(aTableName).value();
      theResult = theStorage.saveEntityBlock(
          theEntity, static_cast<int32_t>(theEntityBlockNum));

      TableFormatter::printStatusRowDuration(
          output, theResult, aRowCollect.size(), Config::getTimer().elapsed());
    }
  }
  return theResult;
}

StatusResult DBProcessor::showIndexes() {
  StatusResult theResult{Errors::noDatabaseInUse};
  if (auto* theActiveDB = app->getDatabaseInUse()) {
    theActiveDB->setDebugInfo("showIndexes");
    std::vector<std::streamsize> theWidths = {10, 10};

    output.setf(std::ios::left, std::ios::adjustfield);
    TableFormatter::printBreak(output, theWidths);
    output.fill(' ');
    output.width(theWidths[0]);
    output << "| table  ";
    output.fill(' ');
    output.width(theWidths[0]);
    output << "| field(s)"
           << "|\n";
    TableFormatter::printBreak(output, theWidths);

    IndexMap& theIndexMap = theActiveDB->getIndexMap();
    for (const auto& [theTableName, theIndex] : theIndexMap) {
      output.fill(' ');
      output.width(theWidths[0]);
      output << "| " + theTableName;
      output.fill(' ');
      output.width(theWidths[0]);
      output << "| " + (theIndex->getName()) << "|\n";
    }

    TableFormatter::printBreak(output, theWidths);
    TableFormatter::printRowsInSet(output, theIndexMap.size());
    TableFormatter::printDuration(output, Config::getTimer().elapsed());
    theResult = Errors::noError;
  }
  return theResult;
}

StatusResult DBProcessor::showIndexFromTable(
    const std::string& aTableName,
    [[maybe_unused]] const StringList& aFieldList) {
  StatusResult theResult{Errors::noDatabaseInUse};
  if (auto* theActiveDB = app->getDatabaseInUse()) {
    theActiveDB->setDebugInfo("show_Index_From");
    std::vector<std::streamsize> theWidths = {20, 20};

    output.setf(std::ios::left, std::ios::adjustfield);
    TableFormatter::printBreak(output, theWidths);
    output.fill(' ');
    output.width(theWidths[0]);
    output << "| key  ";
    output.fill(' ');
    output.width(theWidths[0]);
    output << "| block#"
           << "|\n";
    TableFormatter::printBreak(output, theWidths);

    IndexMap& theIndexMap = theActiveDB->getIndexMap();
    std::unique_ptr<Index>& theIndex = theIndexMap.at(aTableName);
    auto indexVisitor = [&](const IndexKey& theIndexKey,
                            uint32_t theBlockNum) -> bool {
      output.fill(' ');
      output.width(theWidths[0]);
      if (IndexType::intKey == theIndex->getType()) {
        output << "| " + std::to_string(std::get<uint32_t>(theIndexKey));
      } else {
        output << "| " + std::get<std::string>(theIndexKey);
      }
      output.fill(' ');
      output.width(theWidths[0]);
      output << "| " + std::to_string(theBlockNum) << "|\n";
      return true;
    };
    theIndex->eachKV(indexVisitor);

    TableFormatter::printBreak(output, theWidths);
    TableFormatter::printRowsInSet(output, theIndex->getSize());
    TableFormatter::printDuration(output, Config::getTimer().elapsed());
    theResult = Errors::noError;
  }
  return theResult;
}

// ----------------------------------------------------------
Application* DBProcessor::getApp() { return app; }
SQLProcessor& DBProcessor::getSqlProcessor() { return sql; }
auto DBProcessor::getActiveDB() const -> Database* {
  return app->getDatabaseInUse();
}

auto DBProcessor::createEntityFromStream(const std::string& aName) const
    -> Entity {
  if (auto* theActiveDB = getActiveDB()) {
    Index& theEntityIndex = theActiveDB->getEntityIndex();
    if (theActiveDB->entityExistsInDB(aName)) {
      uint32_t theEntityBlkNum = theEntityIndex.valueAt(aName).value();

      std::stringstream theStream;
      // load the entity block into theStream
      StorageInfo theLoadInfo;
      theActiveDB->getStorage().load(theStream, theLoadInfo, theEntityBlkNum,
                                     theActiveDB->getName());
      // decode the stream into an Entity object
      Entity theEntity;
      theEntity.decode(theStream);
      return theEntity;
    }
  }
  return Entity{""};
}

}  // namespace ECE141
