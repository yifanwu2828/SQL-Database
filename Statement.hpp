/**
 * @file Statement.hpp
 * @author Yifan Wu
 * @brief
 * @version 0.3
 * @date 2022-04-17
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef Statement_hpp
#define Statement_hpp

#include <iostream>
#include <string>

#include "keywords.hpp"

namespace ECE141 {
class StatusResult;
class Tokenizer;

enum class StatementType {
  BaseStatement,
  DataBase_Statement,
  SQL_Statement,
};

class Statement {
 public:
  explicit Statement(Keywords aKwType = Keywords::unknown_kw,
            StatementType aStmtType = StatementType::BaseStatement);

  virtual ~Statement()= default;
  virtual StatusResult parse(Tokenizer &aTokenizer);
  virtual StatusResult run(std::ostream &aStream) const;

  [[nodiscard]] virtual Keywords getKwType() const;
  [[nodiscard]] virtual StatementType getStmtType() const;
  [[nodiscard]] virtual std::string getIdentifier() const;
  [[nodiscard]] virtual std::string getStatementName() const;

 protected:
  std::string identifierData;
  Keywords kwType;
  StatementType stmtType;
};

// ------------------------------------------------------------------------------
// 1. RUN {script-file}
class RunScriptStatement : public Statement {
  public:
    explicit RunScriptStatement(Keywords aKwType);
    
    StatusResult parse(Tokenizer& aTokenizer) override;
};

}  // namespace ECE141

#endif /* Statement_hpp */
