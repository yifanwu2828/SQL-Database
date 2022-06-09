/**
 * @file Filters.hpp
 * @author Yifan Wu
 * @brief
 * @version 0.7
 * @date 2022-05-14
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef Filters_h
#define Filters_h

#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include <iosfwd>

#include "BasicTypes.hpp"
#include "Tokenizer.hpp"
#include "keywords.hpp"

namespace ECE141 {

class Entity;
class StatusResult;

bool equals(Value &aLHS, Value &aRHS);
bool notEquals(Value &aLHS, Value &aRHS);
bool lessThan(Value &aLHS, Value &aRHS);
bool lessEquals(Value &aLHS, Value &aRHS);
bool greaterThan(Value &aLHS, Value &aRHS);
bool greaterEquals(Value &aLHS, Value &aRHS);

class Operand {
 public:
  Operand() = default;
  Operand(std::string aName, TokenType aType, Value &aValue, size_t anId = 0);

  TokenType ttype;  // is it a field, or const (#, string)...
  DataTypes dtype;
  std::string name;  // attr name
  Value value;
  size_t entityId{0};
};

//---------------------------------------------------

struct Expression {
  Expression() = default;
  Expression(Operand &aLHSOperand, Operators &anOp, Operand &aRHSOperand,
             std::vector<Logical> &aLogOps);

  bool operator()(const KeyValues &aMap) const;
  bool operator()(const KeyValues &aLHSMap, const KeyValues &aRHSMap) const;
  friend std::ostream &operator<<(std::ostream &aStream,
                                  const Expression &anExpression);

  Operand lhs;                  // id
  Operand rhs;                  // usually a constant; maybe a field...
  Operators op;                 //=     //users.id=books.author_id
  std::vector<Logical> logics;  // and, or, not...
};

using Expressions = std::vector<std::unique_ptr<Expression> >;

//---------------------------------------------------

class Filters {
 public:
  Filters() = default;
  ~Filters() = default;
  // no need to delete expressions, they're unique_ptrs!

  [[nodiscard]] size_t getCount() const;
  [[nodiscard]] size_t getExpressionsNum() const;
  const Expressions &getExpressions();

  [[nodiscard]] bool matches(const KeyValues &aMap) const;
  static bool matches(const Expressions &anExpressions, const KeyValues &aMap);
  static bool matches(const Expressions &anExpressions,
                      const KeyValues &aLHSMap, const KeyValues &aRHSMap);

  Filters &add(Expression *anExpression);
  StatusResult parse(Tokenizer &aTokenizer, const Entity &anEntity);

 protected:
  Expressions expressions;
};

}  // namespace ECE141

#endif /* Filters_h */
