/**
 * @file DBStatement.hpp
 * @author Yifan Wu
 * @brief
 * @version 0.3
 * @date 2022-04-16
 *
 * @copyright Copyright (c) 2022
 */

#ifndef DBStatement_hpp
#define DBStatement_hpp

#include <iosfwd>         // for ostream
#include <string>

#include "Statement.hpp"
#include "keywords.hpp"

namespace ECE141 {

class Application;
class Tokenizer;
class StatusResult;

class DBStatement : public Statement {
 public:
  explicit DBStatement(Application* anApp,
                       Keywords aKeywordType = Keywords::unknown_kw);
  [[nodiscard]] std::string getStatementName() const override;

 protected:
  Application* app{};
};

// ------------------------------------------------------------------------------
// 1. CREATE DATABASE {db-name}
class CreateDBStatement : public DBStatement {
 public:
  explicit CreateDBStatement(Application* anApp);
  static bool recognize(Tokenizer& aTokenizer);
  StatusResult parse(Tokenizer& aTokenizer) override;
  StatusResult run(std::ostream& aStream) const override;
};

// ------------------------------------------------------------------------------
// 2. DROP DATABASE {db-name}
class DropDBStatement : public DBStatement {
 public:
  explicit DropDBStatement(Application* anApp);
  static bool recognize(Tokenizer& aTokenizer);
  StatusResult parse(Tokenizer& aTokenizer) override;
  StatusResult run(std::ostream& aStream) const override;
};
// ------------------------------------------------------------------------------
// 3. SHOW DATABASES
class ShowDBStatement : public DBStatement {
 public:
  explicit ShowDBStatement(Application* anApp);
  static bool recognize(Tokenizer& aTokenizer);
  StatusResult parse(Tokenizer& aTokenizer) override;
  StatusResult run(std::ostream& aStream) const override;
};

// ------------------------------------------------------------------------------
// 4. USE {db-name}
class UseDBStatement : public DBStatement {
 public:
  explicit UseDBStatement(Application* anApp);
  static bool recognize(Tokenizer& aTokenizer);
  StatusResult parse(Tokenizer& aTokenizer) override;
  StatusResult run(std::ostream& aStream) const override;
};

// ------------------------------------------------------------------------------
// 5. DUMP DATABASE {db-name}
class DumpDBStatement : public DBStatement {
 public:
  explicit DumpDBStatement(Application* anApp);
  static bool recognize(Tokenizer& aTokenizer);
  StatusResult parse(Tokenizer& aTokenizer) override;
  StatusResult run(std::ostream& aStream) const override;
};

}  // namespace ECE141
#endif /* DBStatement_hpp */
