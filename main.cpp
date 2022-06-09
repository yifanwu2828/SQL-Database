/**
 * @file main.cpp
 * @author Yifan Wu
 * @brief 
 * @version 0.9
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "TestAutomatic.hpp"
#include "TestManually.hpp"

int main(int argc, const char* argv[]) {
  srand(static_cast<uint32_t>(time(nullptr)));
  constexpr const bool ENABLE_PRINT = false;

  if (argc > 1) {
    std::stringstream theOutput;
    ECE141::TestAutomatic theTests(theOutput);

    using TestCall = std::function<bool()>;
    static std::map<std::string, TestCall> theCalls{
        {"App", [&]() { return theTests.doAppTest(); }},
        {"BlockCache", [&]() { return theTests.doBlockCacheTest(); }},
        {"Compile", [&]() { return theTests.doCompileTest(); }},
        {"Delete", [&]() { return theTests.doDeleteTest(); }},
        {"DropTable", [&]() { return theTests.doDropTest(); }},
        {"DBCommands", [&]() { return theTests.doDBCommandsTest(); }},
        {"Index", [&]() { return theTests.doIndexTest(); }},
        {"Insert", [&]() { return theTests.doInsertTest(); }},
        {"Joins", [&]() { return theTests.doJoinTest(); }},
        {"Reader", [&]() { return theTests.doReaderTest(); }},
        {"RowCache", [&]() { return theTests.doRowCacheTest(); }},
        {"Select", [&]() { return theTests.doSelectTest(); }},
        {"Tables", [&]() { return theTests.doTablesTest(); }},
        {"Update", [&]() { return theTests.doUpdateTest(); }},
        {"ViewCache", [&]() { return theTests.doViewCacheTest(); }},

        // ? custom-------------------------------------------------------
        {"Custom", [&]() { return theTests.doCustomTablesTest(); }},
        {"CustomIndex", [&]() { return theTests.doCustomIndexTest(); }},
        {"DebugTable", [&]() { return theTests.doDebugTablesTest(); }},
        {"LeftJoin", [&]() { return theTests.doCustomLeftJoinTest(); }},
        {"Load", [&]() { return theTests.doCustomLoadTest(); }},
        {"LogicalEdgeSelect",
         [&]() { return theTests.doCustomLogicalSelectEdgeTest(); }},
        {"LogicalSelect", [&]() { return theTests.doLogicSelectTest(); }},
        {"Save", [&]() { return theTests.doCustomSaveTest(); }},
        {"SelfSwitch", [&]() { return theTests.doSelfSwitchDBTest(); }},
        {"Switch", [&]() { return theTests.doCustomSwitchDBTest(); }},

        // All test combined
        {"All", [&]() { return theTests.doALLTest(); }},

    };

    std::string theCmd(argv[1]);
    if (theCalls.count(theCmd)) {
      bool theResult = theCalls[theCmd]();
      const char* theStatus[] = {"FAIL", "PASS"};
      // std::cout << theCmd << " test " << theStatus[theResult] << "\n";
      // std::cout << "------------------------------\n"
      // << theOutput.str() << "\n";
      if ("All" != theCmd || ENABLE_PRINT) {
        std::cout << "------------------------------\n";
        std::cout << theCmd << " test:\n";
        std::cout << theOutput.str() << '\n';
        std::cout << "------------------------------\n";
        std::cout << theCmd << " test " << theStatus[theResult] << '\n';
      }
    } else
      std::cout << "Unknown test\n";
  } else {
    doManualTesting();
  }
  return 0;
}
