/**
 * @file TestAutomatic.hpp
 * @author Yifan Wu
 * @brief 
 * @version 0.9
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef TestAutomatic_h
#define TestAutomatic_h

#include <iostream>
#include <sstream>
#include <vector>
#include <initializer_list>
#include <algorithm>
#include <random>
#include <stack>

#include "Application.hpp"
#include "AboutUs.hpp"
#include "Errors.hpp"
#include "Config.hpp"
#include "FolderReader.hpp"
#include "TestSequencer.hpp"
#include "Faked.hpp"
#include "Timer.hpp"
#include "ScriptRunner.hpp"

//void showErrors(ECE141::StatusResult &aResult, std::ostream &anOutput) {
//
//  static std::map<ECE141::Errors, std::string> theMessages = {
//    {ECE141::illegalIdentifier, "Illegal identifier"},
//    {ECE141::unknownIdentifier, "Unknown identifier"},
//    {ECE141::databaseExists, "Database exists"},
//    {ECE141::tableExists, "Table Exists"},
//    {ECE141::syntaxError, "Syntax Error"},
//    {ECE141::unknownCommand, "Unknown command"},
//    {ECE141::unknownDatabase,"Unknown database"},
//    {ECE141::unknownTable,   "Unknown table"},
//    {ECE141::unknownError,   "Unknown error"}
//  };
//
//  std::string theMessage="Unknown Error";
//  if(theMessages.count(aResult.error)) {
//    theMessage=theMessages[aResult.error];
//  }
//  anOutput << "Error (" << aResult.error << ") " << theMessage << "\n";
//}

namespace ECE141 {
  
  using StringMap = std::map<std::string, std::string>;
  using CountList = std::vector<int>;

//  void showErrors(ECE141::StatusResult &aResult, std::ostream &anOutput) {
//    
//    static std::map<ECE141::Errors, std::string> theMessages = {
//      {ECE141::illegalIdentifier, "Illegal identifier"},
//      {ECE141::unknownIdentifier, "Unknown identifier"},
//      {ECE141::databaseExists, "Database exists"},
//      {ECE141::tableExists, "Table Exists"},
//      {ECE141::syntaxError, "Syntax Error"},
//      {ECE141::unknownCommand, "Unknown command"},
//      {ECE141::unknownDatabase,"Unknown database"},
//      {ECE141::unknownTable,   "Unknown table"},
//      {ECE141::unknownError,   "Unknown error"}
//    };
//
//    std::string theMessage="Unknown Error";
//    if(theMessages.count(aResult.error)) {
//      theMessage=theMessages[aResult.error];
//    }
//    anOutput << "Error (" << aResult.error << ") "
//      << theMessage << "\n";
//  }

  enum Commands {unknown, alter, createDB, createTable, describe,
    delet, dropDB, dropTable, dumpDB, insert, select, showDBs,
    showTables, showIndex, showIndexes, update, useDB};

  struct CommandCount {
    Commands  command;
    int       count;
    char      cmp='=';
    
    bool compare(const CommandCount &anItem) {
      if(command==anItem.command) {
        if('>'==cmp) return anItem.count>count;
        return anItem.count==count;
      }
      return false;
    }
  };

  using Responses = std::vector<CommandCount>;
  using ExpectList = std::initializer_list<CommandCount>;

  class Expected {
  public:
    Expected(const ExpectList &aList) : expected(aList) {}
    
    bool operator==(const Responses &aResponses) {
      size_t theSize=expected.size();
      if(theSize && aResponses.size()==theSize) {
        for(size_t i=0;i<theSize;i++) {
          if(!expected[i].compare(aResponses[i])) {
            return false;
          }
        }
        return true;
      }
      return false;
    }
    
  protected:
    Responses expected;
  };

  class TestAutomatic {
  protected:
    std::ostream &output;
    
  public:
    
    TestAutomatic(std::ostream &anOutput) : output(anOutput) {}
    
    ~TestAutomatic() {std::cout << "Test Version 1.95\n";}
    
    void addUsersTable(std::ostream &anOutput) {
      anOutput << "create table Users (";
      anOutput << " id int NOT NULL auto_increment primary key,";
      anOutput << " first_name varchar(50) NOT NULL,";
      anOutput << " last_name varchar(50),";
      anOutput << " age int,";
      anOutput << " zipcode int);\n";
    }

    void addPaymentsTable(std::ostream &anOutput) {
      anOutput << "create table Payments (";
      anOutput << " id int NOT NULL auto_increment primary key,";
      anOutput << " user_id int,";
      anOutput << " paydate int,";
      anOutput << " amount int);\n";
    }

    void addAccountsTable(std::ostream &anOutput) {
      anOutput << "create table Accounts (";
      anOutput << " id int NOT NULL auto_increment primary key,";
      anOutput << " account_type varchar(25) NOT NULL,";
      anOutput << " amount int);\n";
    }
    
    void addBooksTable(std::ostream &anOutput) {
      anOutput << "create table Books (";
      anOutput << " id int NOT NULL auto_increment primary key,";
      anOutput << " title varchar(25) NOT NULL,";
      anOutput << " subtitle varchar(25),";
      anOutput << " user_id int);\n";
    }
    
    bool doCompileTest() {
      AboutUs theAbout;
      auto theCount=theAbout.getTeamSize();
      std::vector<std::string> theNames;
      for(size_t i=0;i<theCount;i++) {
        if(auto theName=theAbout.getName(i))
          theNames.push_back(*theName);
      }
      return theNames.size()>0;
    }

    StatusResult doScriptTest(std::istream &anInput, std::ostream &anOutput) {
      ECE141::Application theApp(anOutput);
      ScriptRunner        theRunner(theApp);
      return theRunner.run(anInput, anOutput);
    }
    
    //----------------------------------------------

    bool doAppTest() {

      std::string theInput("version;help;quit;");
      std::stringstream theStream(theInput);
      std::stringstream theOutput;
      StatusResult theResult=doScriptTest(theStream,theOutput);
      std::string temp=theOutput.str();
      output << temp << "\n";
      
      if(theResult) {
        const char* theLines[]={
          theInput.c_str(),
          "Version 0.9", "Help system available",
          "DB::141 is shutting down"
        };
        
        std::stringstream theStream(temp);
        std::string theLine;
          
        for(auto *theNext : theLines) {
          std::getline(theStream,theLine);
          if(theLine!=theNext) return false;
        }
        
        return true;
      }
      
      return false;
    }
        
    //---------------------------------------------------
    
    enum states {wasUnknown, wasAlter, wasCreate, wasDescribe,
        wasDelete, wasDrop, wasDump, wasInsert, wasSelect,
        wasShow, wasUpdate, wasUse};
              
    //validates output of DBCommand test..
    size_t analyzeOutput(std::istream &aStream, Responses &aResults) {
      std::stack<Commands> theStack; //tracking open state...

      static KWList createDB{Keywords::create_kw,Keywords::database_kw};
      static KWList showDBs{Keywords::show_kw,Keywords::databases_kw};
      static KWList dumpDB{Keywords::dump_kw,Keywords::database_kw};
      static KWList dropDB{Keywords::drop_kw,Keywords::database_kw};
      static KWList createTable{Keywords::create_kw,Keywords::table_kw};
      static KWList showTables{Keywords::show_kw,Keywords::tables_kw};
      static KWList showIndex{Keywords::show_kw,Keywords::index_kw};
      static KWList showIndexes{Keywords::show_kw,Keywords::indexes_kw};
      static KWList dropTable{Keywords::drop_kw,Keywords::table_kw};
      static KWList insertInto{Keywords::insert_kw,Keywords::into_kw};

      Tokenizer theTokenizer(aStream);
      if(theTokenizer.tokenize()) {
        TestSequencer theSeq(theTokenizer);
        int theValue{0};
        while(theTokenizer.more()) {
          if(theSeq.clear().nextIs(createDB)) {
            if(theSeq.skip(2).nextIs({Keywords::query_kw}).skip(2)) {
              theSeq.getNumber(theValue).skip(7);
              aResults.push_back({Commands::createDB,theValue});
            }
          }
          else if(theSeq.clear().nextIs(showDBs)) {
            if(theSeq.skip(1)) {
              theTokenizer.skipTo(TokenType::number);
              theSeq.getNumber(theValue).skipPast(')');
              aResults.push_back({Commands::showDBs,theValue});
            }
          }
          else if(theSeq.clear().nextIs({Keywords::use_kw})) {
            static KWList dbChanged{Keywords::database_kw,Keywords::changed_kw};
            if(theSeq.skip(2).nextIs(dbChanged)) {
              aResults.push_back({Commands::useDB, 0});
            }
          }
          else if(theSeq.clear().nextIs(dropDB)) {
            if(theSeq.skip(2).nextIs({Keywords::query_kw}).skip(2)) {
              theSeq.getNumber(theValue).skipPast(')');
              aResults.push_back({Commands::dropDB,theValue});
            }
          }
          else if(theSeq.clear().nextIs(dumpDB)) {
            if(theTokenizer.skipTo(Keywords::rows_kw)) {
              auto theToken=theTokenizer.peek(-1);
              theValue=std::stoi(theToken.data);
              theSeq.skip(7);
            }
            aResults.push_back({Commands::dumpDB,theValue});
          }
          else if(theSeq.clear().nextIs(createTable)) {
            if(theSeq.skipPast(';').nextIs({Keywords::query_kw}).skip(2)) {
              theSeq.getNumber(theValue).skip(7);
              aResults.push_back({Commands::createTable,theValue});
            }
          }
          else if(theSeq.clear().nextIs(showTables)) {
            theTokenizer.skipTo(TokenType::number);
            theSeq.getNumber(theValue).skipPast(')');
            aResults.push_back({Commands::showTables,theValue});
          }
          else if(theSeq.clear().nextIs(showIndex)) {
            if(theTokenizer.skipTo(Keywords::rows_kw)) {
              auto theToken=theTokenizer.peek(-1);
              theValue=std::stoi(theToken.data);
              theSeq.skip(7);
            }
            aResults.push_back({Commands::showIndex,theValue});
          }
          else if(theSeq.clear().nextIs(showIndexes)) {
            theTokenizer.skipTo(TokenType::number);
            theSeq.getNumber(theValue).skipPast(')');
            aResults.push_back({Commands::showIndexes,theValue});
          }
          else if(theSeq.clear().nextIs(dropTable)) {
            theTokenizer.skipTo(TokenType::number);
            theSeq.getNumber(theValue).skipPast(')');
            aResults.push_back({Commands::dropTable,theValue});
          }
          else if(theSeq.clear().nextIs(insertInto)) {
            if(theSeq.skipPast(';').nextIs({Keywords::query_kw}).skip(2)) {
              theSeq.getNumber(theValue).skip(7);
              aResults.push_back({Commands::insert,theValue});
            }
          }
          else if(theTokenizer.skipIf(Keywords::describe_kw)) {
            if(theTokenizer.skipTo(Keywords::rows_kw)) {
              auto theToken=theTokenizer.peek(-1);
              theValue=std::stoi(theToken.data);
              aResults.push_back({Commands::describe,theValue});
              theSeq.clear().skipPast(')');
            }
          }
          else if(theSeq.clear().nextIs({Keywords::select_kw})) {
            if(theTokenizer.skipTo(Keywords::rows_kw)) {
              auto theToken=theTokenizer.peek(-1);
              theValue=std::stoi(theToken.data);
              aResults.push_back({Commands::select,theValue});
              theSeq.clear().skipPast(')');
            }
          }
          else if(theSeq.clear().nextIs({Keywords::update_kw})) {
            if(theTokenizer.skipTo(Keywords::rows_kw)) {
              auto theToken=theTokenizer.peek(-1);
              theValue=std::stoi(theToken.data);
              aResults.push_back({Commands::update,theValue});
              theSeq.clear().skipPast(')');
            }
          }
          else if(theSeq.clear().nextIs({Keywords::delete_kw})) {
            if(theTokenizer.skipTo(Keywords::rows_kw)) {
              auto theToken=theTokenizer.peek(-1);
              theValue=std::stoi(theToken.data);
              aResults.push_back({Commands::delet,theValue});
              theSeq.clear().skipPast(')');
            }
          }
          else theTokenizer.next(); //skip...
        }
      }
    return aResults.size();
  }
        
    using FileList = std::vector<std::string>;
    
    bool hasFiles(FileList &aFilelist) {
      for(auto theFile : aFilelist) {
        std::string thePath=Config::getDBPath(theFile);
        std::ifstream theStream(thePath);
        if(!theStream) return false;
      }
      return true;
    }
    
    std::string getRandomDBName(char aChar) {
      uint32_t theCount=rand() % 99999;
      return std::string("testdb_"+std::to_string(theCount+1000)+aChar);
    }
    
    bool compareCounts(CountList &aList1, CountList &aList2, const size_t aPos) {
      bool theResult=true;
      for(size_t i=0;i<aList1.size();i++) {
        if(theResult) {
          theResult=i==aPos ? aList1[i]<aList2[i] : aList1[i]==aList2[i];
        }
      }
      return theResult;
    }
    
    size_t countDBFiles() {
      std::string thePath=Config::getStoragePath();
      FolderReader theReader(thePath.c_str());
      size_t theCount{0};
      theReader.each(Config::getDBExtension(),
                     [&](const std::string &aName) {
        theCount++;
        return true;
      });
      return theCount;
    }
      
   //----------------------------------------------
    bool createFile(const std::string &aPath, const std::string &aName) {
      std::string temp(aPath);
      temp+='/';
      temp+=aName;
      std::fstream output(temp, std::fstream::out | std::fstream::trunc);
      output << "some text\n";
      return true;
    }
    
    //----------------------------------------------

    bool doReaderTest() {
      std::string thePath=Config::getStoragePath();
      createFile(thePath,"test1.txt");
      
      FolderReader theReader(thePath.c_str());
      size_t theCount{0};
      theReader.each(".txt",[&](const std::string &aName) {
        theCount++;
        return true;
      });
      return theCount>0;
    }
    //----------------------------------------------

    bool doDBCommandsTest() {
      
      std::vector<std::string> theFiles;
      
      //size_t theDBCount=countDBFiles();
      
      char theType='A';
      theFiles.push_back(getRandomDBName(theType));
      theFiles.push_back(getRandomDBName(theType));
      theFiles.push_back(getRandomDBName(theType));

      std::stringstream theStream1;
      theStream1 << "CREATE DATABASE "+theFiles[0] << ";\n";
      theStream1 << "create database "+theFiles[1] << ";\n";
      theStream1 << "CrEaTe dAtABaSe "+theFiles[2] << ";\n";
      theStream1 << "use "+theFiles[0] << ";\n";
      theStream1 << "shoW databaseS;\n";
   
      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      std::stringstream theOutput1;
      bool theResult=doScriptTest(theStream1,theOutput1) && hasFiles(theFiles);
      if(theResult) {
        auto temp=theOutput1.str();
        output << temp; //show user...
        //std::cout << temp;
              
        Responses theResponses;
        auto theCount=analyzeOutput(theOutput1,theResponses);
             
        Expected theExpected({
          {Commands::createDB,1}, {Commands::createDB,1},
          {Commands::createDB,1}, {Commands::useDB,0},
          {Commands::showDBs,2,'>'}
        });
     
        if(theCount && theExpected==theResponses) {
          std::stringstream theStream2;
          theStream2 << "DRop dataBASE "+theFiles[1] << ";\n";
          theStream2 << "shoW databaseS;\n";
          theStream2 << "dump database "+theFiles[0] << ";\n";
          theStream2 << "drop database "+theFiles[0] << ";\n";
          theStream2 << "drop database "+theFiles[2] << ";\n";

          std::stringstream theOutput2;
          if((theResult=doScriptTest(theStream2,theOutput2))) {
            temp=theOutput2.str();
            output << temp << "\n"; //DEBUG!
            //std::cout << temp << "\n"; //DEBUG
            std::stringstream theOutput(temp);
  
            int thePrevDBCount=theResponses[4].count;
            Responses theResponses2;

            if((theResult=analyzeOutput(theOutput,theResponses2))) {

              Expected theExpected({
                {Commands::dropDB,0}, {Commands::showDBs,thePrevDBCount-1},
                {Commands::dumpDB,0,'>'}, {Commands::dropDB,0},
                {Commands::dropDB}
              });
           
              if(theExpected==theResponses2) {
                return true;
              }
            }
  
          }
        }
        else theResult=false;

      }
      return theResult;
    }
          
  void insertUsers(std::ostream &anOut, size_t anOffset, size_t aLimit) {
    static const char* kUsers[]={
      " (\"Terry\",     \"Pratchett\", 70,  92124)",
      " (\"Ian\",       \"Tregellis\", 48,  92123)",
      " (\"Jody\",      \"Taylor\",    50,  92120)",
      " (\"Stephen\",   \"King\",      74,  92125)",
      " (\"Ted\",       \"Chiang\",    56,  92120)",
      " (\"Anthony\",   \"Doerr\",     52,  92122)",
      " (\"J.R.R.\",    \"Tolkien\",   130, 92126)",
      " (\"Aurthur C.\",\"Clarke\",    105, 92127)",
      " (\"Seldon\",    \"Edwards\",   81,  92128)",
      " (\"Neal\",      \"Stephenson\",62,  92121)"
    };
    

    anOut<<"INSERT INTO Users (first_name, last_name, age, zipcode)";
    
    size_t theSize=sizeof(kUsers)/sizeof(char*);
    size_t theLimit=std::min(theSize, anOffset+aLimit);
    const char* thePrefix=" VALUES";
    for(size_t i=anOffset;i<theLimit;i++) {
      anOut << thePrefix << kUsers[i];
      thePrefix=",";
    }
    anOut << ";\n";
  }
             
  void insertFakeUsers(std::ostream &anOut,
                       size_t aGroupSize,
                       size_t aGroupCount=1) {
    
    for(size_t theCount=0;theCount<aGroupCount;theCount++) {
      anOut<<"INSERT INTO Users (first_name, last_name, age, zipcode) VALUES ";
      const char* thePrefix="";
      for(size_t theSize=0;theSize<aGroupSize;theSize++) {
        anOut << thePrefix <<
          '(' << '"' << Fake::People::first_name()
          << "\", \"" << Fake::People::last_name()
          << "\", " << Fake::People::age(20,60)
          << ", " << Fake::Places::zipcode() << ')';
        thePrefix=",";
      }
      anOut << ";\n";
    }
  }
    
  void insertBooks(std::ostream &anOut,
                   size_t anOffset, size_t aLimit) {
    static const char* kBooks[]={
      " (\"The Green Mile\",4)",
      " (\"The Stand\",4)",
      " (\"Misery\",4)",
      " (\"11/22/63\",4)",
      " (\"The Institute\",4)",
      " (\"Sorcerer\",1)",
      " (\"Wintersmith\",1)",
      " (\"Mort\",1)",
      " (\"Thud\",1)",
      " (\"Time Police\",3)",
      " (\"The Mechanical\",2)",
      " (\"The Liberation\",2)",
      " (\"The Rising\",2)",
      " (\"Exhalation\",5)",
    };
    
    anOut<<"INSERT INTO Books (title, user_id)";
    
    size_t theSize=sizeof(kBooks)/sizeof(char*);
    size_t theLimit=std::min(theSize, anOffset+aLimit);
    const char* thePrefix=" VALUES";
    for(size_t i=anOffset;i<theLimit;i++) {
      anOut << thePrefix << kBooks[i];
      thePrefix=",";
    }
    anOut << ";\n";
  }
    //----------------------------------------------

    bool doTablesTest() {
      
      std::string theDBName(getRandomDBName('B'));
  
      std::stringstream theStream1;
      theStream1 << "create database " << theDBName << ";\n";
      theStream1 << "use " << theDBName << ";\n";
      
      addUsersTable(theStream1);
      addAccountsTable(theStream1);
      addPaymentsTable(theStream1);
   
      theStream1 << "show tables;\n";
      theStream1 << "describe Accounts;\n";
      theStream1 << "drop table Accounts;\n";
      theStream1 << "show tables;\n";
      theStream1 << "drop database " << theDBName << ";\n";

      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      std::stringstream theOutput1;
      bool theResult=doScriptTest(theInput,theOutput1);
      if(theResult) {
        std::string tempStr=theOutput1.str();
        std::stringstream theOutput2(tempStr);
        output << tempStr << "\n"; //DEBUG!
        
        Responses theResponses;
        auto theCount=analyzeOutput(theOutput1,theResponses);
             
        Expected theExpected({
          {Commands::createDB,1},    {Commands::useDB,0},
          {Commands::createTable,1}, {Commands::createTable,1},
          {Commands::createTable,1}, {Commands::showTables,3},
          {Commands::describe,3},    {Commands::dropTable,1},
          {Commands::showTables,2},  {Commands::dropDB,0},
        });
     
        if(!theCount || !(theExpected==theResponses)) {
          theResult=false;
        }
        
      }
      return theResult;
    }
    
    bool doInsertTest() {
 
      std::string theDBName(getRandomDBName('C'));
  
      std::stringstream theStream1;
      theStream1 << "create database " << theDBName << ";\n";
      theStream1 << "use " << theDBName << ";\n";
      
      addUsersTable(theStream1);
      insertUsers(theStream1,0,5);
      insertFakeUsers(theStream1,50,2);

      theStream1 << "show tables;\n";
      theStream1 << "dump database " << theDBName << ";\n";
      theStream1 << "drop database " << theDBName << ";\n";
            
      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      std::stringstream theOutput;
      bool theResult=doScriptTest(theInput,theOutput);
      if(theResult) {
        std::string tempStr=theOutput.str();
        output << "output \n" << tempStr << "\n";
        //std::cout << tempStr << "\n";
        
        Responses theResponses;
        auto theCount=analyzeOutput(theOutput,theResponses);
             
        Expected theExpected({
          {Commands::createDB,1},    {Commands::useDB,0},
          {Commands::createTable,1}, {Commands::insert,5},
          {Commands::insert,50},     {Commands::insert,50},
          {Commands::showTables,1},  {Commands::dumpDB,3,'>'},
          {Commands::dropDB,0},
        });
     
        if(!theCount || !(theExpected==theResponses)) {
          theResult=false;
        }
   
      }
      return theResult;
    }
    
    bool doUpdateTest() {

      std::string theDBName1(getRandomDBName('E'));
      std::string theDBName2(getRandomDBName('E'));

      std::stringstream theStream1;
      theStream1 << "create database " << theDBName2 << ";\n";
      theStream1 << "create database " << theDBName1 << ";\n";
      theStream1 << "use " << theDBName1 << ";\n";
      
      addUsersTable(theStream1);
      insertUsers(theStream1,0,5);

      theStream1 << "use " << theDBName2 << ";\n";
      theStream1 << "drop database " << theDBName2 << ";\n";
      theStream1 << "use " << theDBName1 << ";\n";

      theStream1 << "select * from Users;\n";

      std::string theZip(std::to_string(10000+rand()%75000));

      theStream1 << "update Users set zipcode=" << theZip
                 << " where id=5;\n";
      
      theStream1 << "select * from Users where zipcode="
                 << theZip << ";\n";
      
      theStream1 << "drop database " << theDBName1 << ";\n";
      theStream1 << "quit;\n";

      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      std::stringstream theOutput;
      bool theResult=doScriptTest(theInput,theOutput);
      if(theResult) {
   
        std::string tempStr=theOutput.str();
        output << "output \n" << tempStr << "\n";
        //std::cout << tempStr << "\n";
        
        Responses theResponses;
        auto theCount=analyzeOutput(theOutput,theResponses);
        
        Expected theExpected({
          {Commands::createDB,1},     {Commands::createDB,1},
          {Commands::useDB,0},        {Commands::createTable,1},
          {Commands::insert,5},       {Commands::useDB,0},
          {Commands::dropDB,0},       {Commands::useDB,0},
          {Commands::select,5},       {Commands::update,1},
          {Commands::select,1},       {Commands::dropDB,0}
        });

        if(!theCount || !(theExpected==theResponses)) {
          theResult=false;
        }
      }
      return theResult;
    }

    bool doDeleteTest() {

      std::string theDBName1(getRandomDBName('F'));
      std::string theDBName2(getRandomDBName('F'));

      std::stringstream theStream1;
      theStream1 << "create database " << theDBName2 << ";\n";
      theStream1 << "create database " << theDBName1 << ";\n";
      theStream1 << "use " << theDBName1 << ";\n";
     
      addUsersTable(theStream1);
      insertUsers(theStream1,0,5);
      
      theStream1 << "use " << theDBName2 << ";\n";
      theStream1 << "drop database " << theDBName2 << ";\n";
      theStream1 << "use " << theDBName1 << ";\n";

      theStream1 << "select * from Users;\n";

      theStream1 << "DELETE from Users where zipcode=92120;\n";
      theStream1 << "select * from Users\n";
      theStream1 << "DELETE from Users where zipcode<92124;\n";
      theStream1 << "select * from Users\n";
      theStream1 << "DELETE from Users where zipcode>92124;\n";
      theStream1 << "select * from Users\n";
      theStream1 << "drop database " << theDBName1 << ";\n";
      theStream1 << "quit;\n";

      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      std::stringstream theOutput;
      bool theResult=doScriptTest(theInput,theOutput);
      if(theResult) {
   
        std::string tempStr=theOutput.str();
        output << "output \n" << tempStr << "\n";
        //std::cout << tempStr << "\n";
        
        Responses theResponses;
        auto theCount=analyzeOutput(theOutput,theResponses);
        
        //XXX-HACK: fix me...
        Expected theExpected({
          {Commands::createDB,1},    {Commands::createDB,1},
          {Commands::useDB,0},       {Commands::createTable,1},
          {Commands::insert,5},      {Commands::useDB,0},
          {Commands::dropDB,0},      {Commands::useDB,0},
          {Commands::select,5},      {Commands::delet,2},
          {Commands::select,3},      {Commands::delet,1},
          {Commands::select,2},      {Commands::delet,1},
          {Commands::select,1},      {Commands::dropDB,0},
        });

        if(!theCount || !(theExpected==theResponses)) {
          theResult=false;
        }
      }
      return theResult;
    }
    
    std::string getUserSelect(const std::initializer_list<std::string> &aClauses) {
      std::string theResult("SELECT * from Users ");
      if(aClauses.size()) {
        std::vector<std::string> theClauses(aClauses);
        auto rd = std::random_device {};
        auto rng = std::default_random_engine {rd()};
        std::shuffle(theClauses.begin(), theClauses.end(), rng);
        for(auto theClause : theClauses) {
          theResult+=theClause;
        }
      }
      theResult+=";\n";
      return theResult;
    }
    
    bool doSelectTest() {

      std::stringstream theStream1;
      std::string theDBName("db_"+std::to_string(rand()%9999));
      theStream1 << "create database " << theDBName << ";\n";
      theStream1 << "use " << theDBName << ";\n";
      
      addUsersTable(theStream1);
      insertUsers(theStream1,0,10);

      theStream1 << getUserSelect({});//basic
      theStream1 << getUserSelect({" order by zipcode"," where zipcode>92122"," limit 3"});
      theStream1 << "select first_name, last_name, age from Users order by last_name where age>60;\n";

      theStream1 << "show tables;\n";
      theStream1 << "dump database " << theDBName << ";\n";
      theStream1 << "drop database " << theDBName << ";\n";
      
      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      std::stringstream theOutput;
      bool theResult=doScriptTest(theInput,theOutput);
      if(theResult) {
   
        std::string tempStr=theOutput.str();
        output << "output \n" << tempStr << "\n";
       // std::cout << tempStr << "\n";
        
        Responses theResponses;
        auto theCount=analyzeOutput(theOutput,theResponses);
        
        Expected theExpected({
          {Commands::createDB,1},    {Commands::useDB,0},
          {Commands::createTable,1}, {Commands::insert,10},
          {Commands::select,10},     {Commands::select,3},
          {Commands::select,6},
          {Commands::showTables,1},
          {Commands::dumpDB,3,'>'},  {Commands::dropDB,0},
        });

        if(!theCount || !(theExpected==theResponses)) {
          theResult=false;
        }
                           
      }
      return theResult;
    }

    //test dropping a table...
    bool doDropTest() {
 
      std::string theDBName1(getRandomDBName('G'));

      std::stringstream theStream1;
      theStream1 << "create database " << theDBName1 << ";\n";
      theStream1 << "use " << theDBName1 << ";\n";
      
      addAccountsTable(theStream1);
      addUsersTable(theStream1);
      insertUsers(theStream1,0,5);

      theStream1 << "show tables\n";
      theStream1 << "drop table Users;\n";
      theStream1 << "show tables\n";
      theStream1 << "drop database " << theDBName1 << ";\n";
      
      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      std::stringstream theOutput;
      
      bool theResult=doScriptTest(theInput,theOutput);      
      if(theResult) {
        std::string tempStr=theOutput.str();
        output << "output \n" << tempStr << "\n";
        //std::cout << tempStr << "\n";
        
        Responses theResponses;
        auto theCount=analyzeOutput(theOutput,theResponses);
        
        Expected theExpected({
          {Commands::createDB,1},    {Commands::useDB,0},
          {Commands::createTable,1}, {Commands::createTable,1},
          {Commands::insert,5},      {Commands::showTables,2},
          {Commands::dropTable,6},   {Commands::showTables,1},
          {Commands::dropDB,0},
        });

        if(!theCount || !(theExpected==theResponses)) {
          theResult=false;
        }
      }
      return theResult;
    }
        
    bool doIndexTest() {
      std::string theDBName1(getRandomDBName('H'));
      std::string theDBName2(getRandomDBName('H'));
      
      //theDBName1="bar";
      std::stringstream theStream1;
      theStream1 << "create database " << theDBName2 << ";\n";
      theStream1 << "create database " << theDBName1 << ";\n";
      theStream1 << "use " << theDBName1 << ";\n";

      addUsersTable(theStream1);
      addBooksTable(theStream1);
      insertUsers(theStream1,0,5);
      insertBooks(theStream1,0,14);
      
      theStream1 << "show indexes;\n";
      theStream1 << "drop table Books;\n";
      theStream1 << "show indexes;\n";

      theStream1 << "use " << theDBName2 << ";\n";
      theStream1 << "drop database " << theDBName2 << ";\n";
      theStream1 << "use " << theDBName1 << ";\n";

      theStream1 << "select * from Users\n";
      insertFakeUsers(theStream1,30,1);
      
      theStream1 << "DELETE from Users where age>60;\n";
      theStream1 << "select * from Users\n";
      insertFakeUsers(theStream1,30,1);

      theStream1 << "show index id from Users;\n";
      theStream1 << "drop database " << theDBName1 << ";\n";
      theStream1 << "quit;\n";

      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      std::stringstream theOutput;
      
      bool theResult=doScriptTest(theInput,theOutput);
      if(theResult) {
        std::string tempStr=theOutput.str();
        output << "output \n" << tempStr << "\n";
        //std::cout << tempStr << "\n";
        Responses theResponses;
        auto theCount=analyzeOutput(theOutput,theResponses);
        
        Expected theExpected({
          {Commands::createDB,1},    {Commands::createDB,1},
          {Commands::useDB,0},       {Commands::createTable,1},
          {Commands::createTable,1}, {Commands::insert,5},
          {Commands::insert,14},     {Commands::showIndexes,2},
          {Commands::dropTable,15},  {Commands::showIndexes,1},
          {Commands::useDB,0},       {Commands::dropDB,0},
          {Commands::useDB,0},       {Commands::select,5},
          {Commands::insert,30},     {Commands::delet,2},
          {Commands::select,33},     {Commands::insert,30},
          {Commands::showIndex,63},  {Commands::dropDB,0},
        });

        if(!theCount || !(theExpected==theResponses)) {
          theResult=false;
        }
        
      }
      return theResult;
    }
    
        
    bool doJoinTest() {

      std::string theDBName1(getRandomDBName('J'));
      std::stringstream theStream1;
      theStream1 << "create database " << theDBName1 << ";\n";
      theStream1 << "use " << theDBName1 << ";\n";
      
      addUsersTable(theStream1);
      insertUsers(theStream1,0,6);
      addBooksTable(theStream1);
      insertBooks(theStream1,0,14);

      theStream1 << "select * from Users order by last_name;\n";

      theStream1 << "select * from Books order by title;\n";

      theStream1 << "select first_name, last_name, title from Users left join Books on Users.id=Books.user_id order by last_name;\n";
      
      theStream1 << "drop database " << theDBName1 << ";\n";
      theStream1 << "quit;\n";
      
      std::stringstream theInput(theStream1.str());
      std::stringstream theOutput;
      
      bool theResult=doScriptTest(theInput,theOutput);
      if(theResult) {
        std::string tempStr=theOutput.str();
        output << "output \n" << tempStr << "\n";
        //std::cout << tempStr << "\n";
        
        Responses theResponses;
        size_t theCount=analyzeOutput(theOutput,theResponses);
        Expected theExpected({
          {Commands::createDB,1},    {Commands::useDB,0},
          {Commands::createTable,1}, {Commands::insert,6},
          {Commands::createTable,1}, {Commands::insert,14},
          {Commands::select,6},      {Commands::select,14},
          {Commands::select,15},     {Commands::dropDB,0},
        });
        if(!theCount || !(theExpected==theResponses)) {
          theResult=false;
        }
      }
      return theResult;
      
    }

    bool doIOTest(double &anElapsed, char aPrefix) {
  
      std::string theDBName1(getRandomDBName(aPrefix));
      std::stringstream theStream1;
      theStream1 << "create database " << theDBName1 << ";\n";
      theStream1 << "use " << theDBName1 << ";\n";
      
      addUsersTable(theStream1);
      insertUsers(theStream1, 0, 10);
      insertFakeUsers(theStream1,50,4);

      theStream1 << "select * from Users;\n";
      theStream1 << "delete from Users where age>50;\n";
      theStream1 << "select * from Users;\n";
      theStream1 << "select * from Users;\n"; //caches should help here...
      theStream1 << "drop database " << theDBName1 << ";\n";
      theStream1 << "quit;\n";
      
      std::stringstream theInput(theStream1.str());
      std::stringstream theOutput;

      Timer theTimer;

      bool theResult=doScriptTest(theInput,theOutput);
      anElapsed=theTimer.elapsed();
      
      if(theResult) {
        std::string tempStr=theOutput.str();
        output << "output \n" << tempStr << "\n";
        //std::cout << tempStr << "\n";
        
        Responses theResponses;
        size_t theCount=analyzeOutput(theOutput,theResponses);
        Expected theExpected({
          {Commands::createDB,1},    {Commands::useDB,0},
          {Commands::createTable,1}, {Commands::insert,10},
          {Commands::insert,50},     {Commands::insert,50},
          {Commands::insert,50},     {Commands::insert,50},
          {Commands::select,210},    {Commands::delet,1,'>'},
          {Commands::select,5,'>'},  {Commands::select,5,'>'},
          {Commands::dropDB,0},
        });
        if(!theCount || !(theExpected==theResponses)) {
          theResult=false;
        }
      }
      return theResult;
    }
    
    bool doCacheTest(CacheType aType, size_t aCapacity) {
      
      //first-- let's test without the cache...
      Config::setCacheSize(aType, 0);
      char  theChar{'P'};
      
      std::vector<double> theTimes;
      bool theResult{true};
      for(size_t i=0;i<5;i++) {
        if(theResult) {
          double theTime{0.0};
          if((theResult=doIOTest(theTime,theChar++))) {
            theTimes.push_back(theTime);
          }
        }
      }

      if(theResult) {
        double theAvgTime=std::accumulate(
                theTimes.begin(), theTimes.end(), 0.0) / theTimes.size();
        
        Config::setCacheSize(aType, aCapacity);
        theTimes.clear();
        for(size_t i=0;i<5;i++) {
          if(theResult) {
            double theTime2{0.0};
            if((theResult=doIOTest(theTime2,theChar++))) {
              theTimes.push_back(theTime2);
            }
          }
        }
        
        if(theResult) {
          double theAvgTime2=std::accumulate(
                  theTimes.begin(), theTimes.end(), 0.0) / theTimes.size();
          return theAvgTime2<theAvgTime;
        }
      }
      
      return theResult;
    }

    bool doBlockCacheTest() {
      return doCacheTest(CacheType::block, 200);
    }

    bool doRowCacheTest() {
      return doCacheTest(CacheType::rows, 200);
    }

    bool doViewCacheTest() {
      return doCacheTest(CacheType::views, 30);
    }
      
  bool doCustomTablesTest() {
      std::string theDBName("CustomDB");
      std::stringstream theStream1;
      theStream1 << "\ncreate database " << theDBName << ";\n";
      theStream1 << "\nuse " << theDBName << ";\n";

      addUsersTable(theStream1);
      addAccountsTable(theStream1);
      theStream1 << "\nshow tables;\n";
      theStream1 << "\ndescribe Accounts;\n";
      theStream1 << "\ndescribe Users;\n";
      theStream1 << "\ndrop table Accounts;\n";
      theStream1 << "\nshow tables;\n";
      theStream1 << "\ndrop database " << theDBName << ";\n";
      theStream1 << "\nquit;\n";

      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      std::stringstream theOutput1;
      bool theResult = doScriptTest(theInput, theOutput1);
      // std::string tempStr=theOutput1.str();
      // std::stringstream theOutput2(tempStr);
      // std::cout << tempStr << "\n";

      return theResult;
    }

    bool doCustomSaveTest() {
      std::string theDBName("CustomDB");
      std::stringstream theStream1;
      theStream1 << "\ncreate database " << theDBName << ";\n";
      theStream1 << "\nuse " << theDBName << ";\n";

      addUsersTable(theStream1);
      addAccountsTable(theStream1);
      theStream1 << "\nshow tables;\n";
      theStream1 << "\ndescribe Users;\n";
      theStream1 << "\nquit;\n";

      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      std::stringstream theOutput1;
      bool theResult = doScriptTest(theInput, theOutput1);
      // std::string tempStr=theOutput1.str();
      // std::stringstream theOutput2(tempStr);
      // std::cout << tempStr << "\n";  // * Uncomment if Debug

      return theResult;
    }

    bool doCustomLoadTest() {
      std::string theDBName("CustomDB");
      std::stringstream theStream1;
      theStream1 << "\nuse " << theDBName << ";\n";

      theStream1 << "\nshow tables;\n";
      theStream1 << "\ndescribe Users;\n";
      theStream1 << "\ndrop database " << theDBName << ";\n";
      theStream1 << "\nquit;\n";

      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      std::stringstream theOutput1;
      bool theResult = doScriptTest(theInput, theOutput1);
      // std::string tempStr=theOutput1.str();
      // std::stringstream theOutput2(tempStr);
      // std::cout << tempStr << "\n"; // * Uncomment if Debug

      return theResult;
    }

    bool doCustomSwitchDBTest() {
      std::string theDBName1("foo");
      std::string theDBName2("bar");
      std::stringstream theStream1;

      // create 2 db
      theStream1 << "\ncreate database " << theDBName1 << ";\n";
      theStream1 << "\ncreate database " << theDBName2 << ";\n";

      // add table to db1
      theStream1 << "\nuse " << theDBName1 << ";\n";
      addUsersTable(theStream1);
      addAccountsTable(theStream1);

      // add table to db2
      theStream1 << "\nuse " << theDBName2 << ";\n";
      addUsersTable(theStream1);
      addAccountsTable(theStream1);

      // switch db
      theStream1 << "\nuse " << theDBName2 << ";\n";
      theStream1 << "\ndrop table Accounts;\n";

      theStream1 << "\nuse " << theDBName1 << ";\n";
      theStream1 << "\ndrop table Users;\n";

      theStream1 << "\nuse " << theDBName2 << ";\n";
      theStream1 << "\nshow tables;\n";
      theStream1 << "\ndescribe Users;\n";

      theStream1 << "\nuse " << theDBName1 << ";\n";
      theStream1 << "\nshow tables;\n";
      theStream1 << "\ndescribe Accounts;\n";

      // at begin db1 {1, 2}  db2 {1, 2}   -> db1 {2} and db2 {1}
      // at this point two db should switch the content

      theStream1 << "\ndump database " << theDBName1 << ";\n";
      theStream1 << "\ndump database " << theDBName2 << ";\n";

      // clean up
      theStream1 << "\ndrop database " << theDBName1 << ";\n";
      theStream1 << "\ndrop database " << theDBName2 << ";\n";
      theStream1 << "\nquit;\n";

      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      std::stringstream theOutput1;
      bool theResult = doScriptTest(theInput, theOutput1);
      // std::string tempStr=theOutput1.str();
      // std::stringstream theOutput2(tempStr);
      // std::cout << tempStr << "\n"; // * Uncomment if Debug

      return theResult;
    }

    bool doSelfSwitchDBTest() {
      std::string theDBName1("fuzz");
      std::stringstream theStream1;

      // create 2 db
      theStream1 << "\ncreate database " << theDBName1 << ";\n";
      theStream1 << "\nuse " << theDBName1 << ";\n";

      // add table to db1
      addUsersTable(theStream1);
      theStream1 << "\nshow tables;\n";
      theStream1 << "\ndescribe Users;\n";

      // switch db
      theStream1 << "\nuse " << theDBName1 << ";\n";
      theStream1 << "\nuse " << theDBName1 << ";\n";
      theStream1 << "\nshow tables;\n";
      theStream1 << "\ndescribe Users;\n";

      // clean up
      theStream1 << "\ndrop database " << theDBName1 << ";\n";
      theStream1 << "\nquit;\n";

      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      std::stringstream theOutput1;
      bool theResult = doScriptTest(theInput, theOutput1);
      // std::string tempStr=theOutput1.str();
      // std::stringstream theOutput2(tempStr);
      // std::cout << tempStr << "\n"; // * Uncomment if Debug

      return theResult;
    }

    bool doDebugTablesTest() {
      std::string theDBName("Debug");

      std::stringstream theStream1;
      theStream1 << "create database " << theDBName << ";\n";
      theStream1 << "use " << theDBName << ";\n";

      addUsersTable(theStream1);
      addAccountsTable(theStream1);
      addPaymentsTable(theStream1);

      theStream1 << "show tables;\n";
      theStream1 << "describe Accounts;\n";
      theStream1 << "drop table Accounts;\n";
      theStream1 << "show tables;\n";
      theStream1 << "drop database " << theDBName << ";\n";

      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      std::stringstream theOutput1;
      bool theResult = doScriptTest(theInput, theOutput1);
      if (theResult) {
        // std::string tempStr=theOutput1.str();
        // std::stringstream theOutput2(tempStr);
        // output << "\n" << tempStr << "\n"; //DEBUG!

        Responses theResponses;
        auto theCount = analyzeOutput(theOutput1, theResponses);

        Expected theExpected({
            {Commands::createDB, 1},
            {Commands::useDB, 1},
            {Commands::createTable, 1},
            {Commands::createTable, 1},
            {Commands::createTable, 1},
            {Commands::showTables, 3},
            {Commands::describe, 3},
            {Commands::dropTable, 1},
            {Commands::showTables, 3},
            {Commands::dropDB, 1},
        });

        if (!theCount || !(theExpected == theResponses)) {
          theResult = false;
        }
      }
      return theResult;
    }

    // Insert
    bool doCustomIndexTest() {
      std::string theDBName("FOO");
      std::string theDBName2("Dummy");

      std::stringstream theStream1;
      theStream1 << "create database " << theDBName << ";\n";
      theStream1 << "create database " << theDBName2 << ";\n";
      theStream1 << "use " << theDBName << ";\n";

      addUsersTable(theStream1);
      insertUsers(theStream1, 0, 2);
      addAuthorsTable(theStream1);
      insertAuthors(theStream1, 0, 3);

      theStream1 << "use " << theDBName2 << ";\n";
      theStream1 << "use " << theDBName << ";\n";

      theStream1 << "dump database " << theDBName << ";\n";
      theStream1 << "show indexes;\n";
      theStream1 << "drop database " << theDBName << ";\n";
      theStream1 << "drop database " << theDBName2 << ";\n";

      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      std::stringstream theOutput;
      bool theResult = doScriptTest(theInput, theOutput);
      if (theResult) {
        std::string tempStr = theOutput.str();
        output << "output \n" << tempStr << "\n";

        Responses theResponses;
        auto theCount = analyzeOutput(theOutput, theResponses);

        Expected theExpected({
            {Commands::createDB, 1},
            {Commands::createDB, 1},
            {Commands::useDB, 0},
            {Commands::createTable, 1},
            {Commands::insert, 2},
            {Commands::createTable, 1},
            {Commands::insert, 3},
            {Commands::useDB, 0},
            {Commands::useDB, 0},
            {Commands::dumpDB, 10, '>'},
            {Commands::showIndexes, 2},
            {Commands::dropDB, 0},
            {Commands::dropDB, 0},
        });

        if (!theCount || !(theExpected == theResponses)) {
          theResult = false;
        }
      }
      return theResult;
    }
    // check Invalid attributes

    bool doCustomSelectTest() {
      std::stringstream theStream1;
      std::string theDBName("db_" + std::to_string(rand() % 9999));
      theStream1 << "create database " << theDBName << ";\n";
      theStream1 << "use " << theDBName << ";\n";

      addUsersTable(theStream1);
      insertUsers(theStream1, 0, 10);

      theStream1 << getUserSelect({});  // basic
      theStream1 << getUserSelect(
          {" order by zipcode", " where zipcode>92122", " limit 3"});
      theStream1 << "select first_name, last_name from Users order by last_name "
                    "where age>60;\n";

      theStream1 << "show tables;\n";
      theStream1 << "dump database " << theDBName << ";\n";
      theStream1 << "drop database " << theDBName << ";\n";

      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      std::stringstream theOutput;
      bool theResult = doScriptTest(theInput, theOutput);
      if (theResult) {
        // std::string tempStr=theOutput.str();
        // output << "output \n" << tempStr << "\n";
        // std::cout << tempStr << "\n";

        Responses theResponses;
        auto theCount = analyzeOutput(theOutput, theResponses);

        Expected theExpected({
            {Commands::createDB, 1},
            {Commands::useDB, 1},
            {Commands::createTable, 1},
            {Commands::insert, 10},
            {Commands::select, 10},
            {Commands::select, 3},
            {Commands::select, 6},
            {Commands::showTables, 1},
            {Commands::dumpDB, 3, '>'},
            {Commands::dropDB, 0},
        });

        if (!theCount || !(theExpected == theResponses)) {
          theResult = false;
        }
      }
      return theResult;
    }

    bool doLogicSelectTest() {
      std::stringstream theStream1;
      std::string theDBName("db_" + std::to_string(rand() % 9999));
      theStream1 << "create database " << theDBName << ";\n";
      theStream1 << "use " << theDBName << ";\n";

      addUsersTable(theStream1);
      insertUsers(theStream1, 0, 10);

      // theStream1 << getUserSelect({});//basic
      theStream1 << "SELECT * from Users order by age where age <=70;\n";
      theStream1 << "select age, first_name, zipcode from Users order by age "
                    "where age <=70 AND zipcode = 92120;\n";
      theStream1 << "select age, first_name, zipcode from Users order by age "
                    "where NOT age > 70 AND zipcode = 92120;\n";

      theStream1 << "show tables;\n";
      theStream1 << "dump database " << theDBName << ";\n";
      theStream1 << "drop database " << theDBName << ";\n";

      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      std::stringstream theOutput;
      bool theResult = doScriptTest(theInput, theOutput);
      if (theResult) {
        // std::string tempStr=theOutput.str();
        // output << "output \n" << tempStr << "\n";
        // std::cout << tempStr << "\n";

        Responses theResponses;
        auto theCount = analyzeOutput(theOutput, theResponses);

        Expected theExpected({
            {Commands::createDB, 1},
            {Commands::useDB, 0},
            {Commands::createTable, 1},
            {Commands::insert, 10},
            {Commands::select, 6},
            {Commands::select, 2},
            {Commands::select, 2},
            {Commands::showTables, 1},
            {Commands::dumpDB, 3, '>'},
            {Commands::dropDB, 0},
        });

        if (!theCount || !(theExpected == theResponses)) {
          theResult = false;
        }
      }
      return theResult;
    }

    bool doCustomLogicalSelectEdgeTest() {
      std::stringstream theStream1;
      std::string theDBName("EdgeSelect");
      theStream1 << "create database " << theDBName << ";\n";
      theStream1 << "use " << theDBName << ";\n";

      addUsersTable(theStream1);
      insertUsers(theStream1, 0, 10);

      theStream1 << "Select * From Users;\n";
      theStream1 << "SELECT age, first_name, zipcode from Users order by age "
                    "where age = 70 AND zipcode = 92124 OR first_name = Ian AND "
                    "last_name = Tregellis;\n";
      theStream1 << "select age, zipcode from Users where NOT age <= 70 OR NOT "
                    "zipcode != 92120 order by age;\n";

      theStream1 << "show tables;\n";
      theStream1 << "dump database " << theDBName << ";\n";
      theStream1 << "drop database " << theDBName << ";\n";

      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      std::stringstream theOutput;
      bool theResult = doScriptTest(theInput, theOutput);
      if (theResult) {
        std::string tempStr = theOutput.str();
        output << "output \n" << tempStr << "\n";
        // std::cout << tempStr << "\n";

        Responses theResponses;
        auto theCount = analyzeOutput(theOutput, theResponses);

        Expected theExpected({
            {Commands::createDB, 1},
            {Commands::useDB, 0},
            {Commands::createTable, 1},
            {Commands::insert, 10},
            {Commands::select, 10},
            {Commands::select, 2},
            {Commands::select, 6},
            {Commands::showTables, 1},
            {Commands::dumpDB, 3, '>'},
            {Commands::dropDB, 0},
        });

        if (!theCount || !(theExpected == theResponses)) {
          theResult = false;
        }
      }
      return theResult;
    }

    // ---------------------------------------------------------------------------
    void addAuthorsTable(std::ostream &anOutput) {
      anOutput << "create table Authors (";
      anOutput << " id int NOT NULL auto_increment primary key,";
      anOutput << " first_name varchar(50) NOT NULL,";
      anOutput << " last_name varchar(50));\n";
    }

    void insertAuthors(std::ostream &anOut, size_t anOffset, size_t aLimit) {
      static const char *kUsers[] = {
          " (Stephen, King)",
          " (JK,      Rowling)",
          " (Truong,  Nguyen)",
      };

      anOut << "INSERT INTO Authors (first_name, last_name)";

      size_t theSize = sizeof(kUsers) / sizeof(char *);
      size_t theLimit = std::min(theSize, anOffset + aLimit);
      const char *thePrefix = " VALUES";
      for (size_t i = anOffset; i < theLimit; i++) {
        anOut << thePrefix << kUsers[i];
        thePrefix = ",";
      }
      anOut << ";\n";
    }
    // ---------------------------------------------------------------------------
    void addHPBooksTable(std::ostream &anOutput) {
      anOutput << "create table Books (";
      anOutput << " id int NOT NULL auto_increment primary key,";
      anOutput << " title varchar(90) NOT NULL,";
      anOutput << " author_id int);\n";
    }

    void insertHPBooks(std::ostream &anOut, size_t anOffset, size_t aLimit) {
      static const char *kHPBooks[] = {
          "(\"Harry Potter and the Sorcerer Stone\", 2)",
          "(\"Harry Potter and the Philosopher Stone\", 2)",
          "(\"Harry Potter and the Prisoner of Azkaban\", 2)",
          "(\"Harry Potter and the Chamber of Secrets\", 2)",
          "(\"Harry Potter and the Goblet of Fire\", 2)",
          "(\"Harry Potter and the Order of the Phoenix\", 2)",
          "(\"Harry Potter and the Half Blood Prince\", 2)",
          "(\"Carrie\", 1)",
          "(\"The Dark Tower\", 1)",
          "(\"The Green Mile\", 1)",
          "(\"Wavelets and Filter Banks\", 0)",
      };
      anOut << "INSERT INTO Books (title, author_id)";

      size_t theSize = sizeof(kHPBooks) / sizeof(char *);
      size_t theLimit = std::min(theSize, anOffset + aLimit);
      const char *thePrefix = " VALUES";
      for (size_t i = anOffset; i < theLimit; i++) {
        anOut << thePrefix << kHPBooks[i];
        thePrefix = ",";
      }
      anOut << ";\n";
    }
    // ---------------------------------------------------------------------------

    bool doCustomLeftJoinTest() {
      std::string theDBName1("LeftJoin");
      std::stringstream theStream1;
      theStream1 << "create database " << theDBName1 << ";\n";
      theStream1 << "use " << theDBName1 << ";\n";

      addAuthorsTable(theStream1);
      insertAuthors(theStream1, 0, 3);

      addHPBooksTable(theStream1);
      insertHPBooks(theStream1, 0, 11);

      theStream1 << "select * from Authors;\n";
      theStream1 << "select * from Books;\n";

      theStream1 << "select last_name, title from Authors left join Books on "
                    "Authors.id=Books.author_id;\n";

      theStream1 << "drop database " << theDBName1 << ";\n";
      theStream1 << "quit;\n";

      std::stringstream theInput(theStream1.str());
      std::stringstream theOutput;

      bool theResult = doScriptTest(theInput, theOutput);
      if (theResult) {
        // std::string tempStr=theOutput.str();
        // output << "output \n" << tempStr << "\n";

        Responses theResponses;
        size_t theCount = analyzeOutput(theOutput, theResponses);
        Expected theExpected({
            {Commands::createDB, 1},
            {Commands::useDB, 0},
            {Commands::createTable, 1},
            {Commands::insert, 3},
            {Commands::createTable, 1},
            {Commands::insert, 11},
            {Commands::select, 3},
            {Commands::select, 11},
            {Commands::select, 11},
            {Commands::dropDB, 0},
        });
        if (!theCount || !(theExpected == theResponses)) {
          theResult = false;
        }
      }
      return theResult;
    }

    // ----------------------------------------------------
    bool doALLTest() {
      using TestCall = std::function<bool()>;
      static std::map<std::string, TestCall> theGivenCalls{
         {"App", [&]() { return doAppTest(); }},
          {"BlockCache", [&]() { return doBlockCacheTest(); }},
          {"Compile", [&]() { return doCompileTest(); }},
          {"Delete", [&]() { return doDeleteTest(); }},
          {"DropTable", [&]() { return doDropTest(); }},
          {"DBCommands", [&]() { return doDBCommandsTest(); }},
          {"Index", [&]() { return doIndexTest(); }},
          {"Insert", [&]() { return doInsertTest(); }},
          {"Joins", [&]() { return doJoinTest(); }},
          {"Reader", [&]() { return doReaderTest(); }},
          {"RowCache", [&]() { return doRowCacheTest(); }},
          {"Select", [&]() { return doSelectTest(); }},
          {"Tables", [&]() { return doTablesTest(); }},
          {"Update", [&]() { return doUpdateTest(); }},
          {"ViewCache", [&]() { return doViewCacheTest(); }},
      };

      static std::map<std::string, TestCall> theCustomCalls{
          {"CustomIndex", [&]() { return doCustomIndexTest(); }},
          // {"LeftJoin", [&]() { return doCustomLeftJoinTest(); }},
          {"LogicalEdgeSelect",
           [&]() { return doCustomLogicalSelectEdgeTest(); }},
          {"LogicalSelect", [&]() { return doLogicSelectTest(); }},
          {"Save", [&]() { return doCustomSaveTest(); }},
          {"SaveAndLoad", [&]() { return doCustomLoadTest(); }},
          {"SelfSwitch", [&]() { return doSelfSwitchDBTest(); }},
          {"Switch", [&]() { return doCustomSwitchDBTest(); }},
      };
      std::vector<std::pair<std::string, std::string>> theCustomMessages;

  #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
      std::array<std::string, 2> theStatus{"FAIL", "PASS"};

  #elif __APPLE__ || defined __linux__ || defined __unix__
      Color color;
      std::array<std::string, 2> theStatus{color.toRed("FAIL"),
                                           color.toGreen("PASS")};

  #endif

      for (const auto &[testName, test] : theCustomCalls) {
        std::string msg = theStatus.at(static_cast<size_t>(test()));
        theCustomMessages.emplace_back(testName, msg);
      }

      // output
      std::cout << "+-----------------------------+\n";
      std::vector<std::pair<std::string, std::string>> theGivenMessages;
      for (const auto &[testName, test] : theGivenCalls) {
        std::string msg = theStatus.at(static_cast<size_t>(test()));
        theGivenMessages.emplace_back(testName, msg);
      }
      for (const auto &[name, res] : theGivenMessages) {
        std::cout << "| " << name << std::right << std::setfill('.')
                  << std::setw(15) << ' ' << res << std::endl;
      }
      std::cout << "+-----------------------------+\n";
      for (const auto &[name, res] : theCustomMessages) {
        std::cout << "| " << name << std::right << std::setfill('.')
                  << std::setw(15) << ' ' << res << std::endl;
      }
      std::cout << "+-----------------------------+\n";
      return true;
    }
  };

}


#endif /* TestAutomatic_h */
