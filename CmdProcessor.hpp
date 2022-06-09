/**
 * @file CmdProcessor.hpp
 * @author Yifan Wu
 * @brief 
 * @version 0.4
 * @date 2022-04-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef CmdProcessor_hpp
#define CmdProcessor_hpp

#include <string>
#include <iosfwd>

namespace ECE141 {

class Statement;
class Tokenizer;
class StatusResult;

class CmdProcessor {  // processor interface
 public:
  explicit CmdProcessor(std::ostream &anOutput, std::string aName = "CmdProcessor");
  virtual ~CmdProcessor() = default;

  CmdProcessor(const CmdProcessor &aCopy) = delete;
  CmdProcessor(CmdProcessor &&aSrc) = delete;
  auto operator=(const CmdProcessor &aCopy) -> CmdProcessor & = delete;
  auto operator=(CmdProcessor &&aSrc) -> CmdProcessor & = delete;

  virtual auto recognizes(Tokenizer &aTokenizer) -> CmdProcessor * = 0;
  virtual auto makeStatement(Tokenizer &aTokenizer, StatusResult &aResult)
      -> Statement * = 0;
  virtual auto run(Statement *aStmt) -> StatusResult = 0;

  std::ostream &output;
  std::string name;
};

}  // namespace ECE141

#endif /* CmdProcessor */
