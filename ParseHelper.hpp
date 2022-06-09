/**
 * @file ParseHelper.hpp
 * @author Yifan Wu
 * @brief 
 * @version 0.1
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef ParseHelper_hpp
#define ParseHelper_hpp

#include <string>

#include "BasicTypes.hpp"
#include "keywords.hpp"

namespace ECE141 {

//-------------------------------------------------
class Attribute;
class Entity;
class Tokenizer;
class StatusResult;
class Operand;

struct ParseHelper {
  explicit ParseHelper(Tokenizer &aTokenizer);
  
  StatusResult parseAttributeOptions(Attribute &anAttribute);
  StatusResult parseAttribute(Attribute &anAttribute);
  StatusResult parseIdentifierList(StringList &aList);
  StatusResult parseKeyValues(KeyValues &aList, Entity &anEntity);
  StatusResult parseValueList(StringList &aList);
  // parse according to index
  StatusResult parseSecond(std::string &aStr);
  StatusResult parseThird(std::string &aStr);

  // expression related
  StatusResult parseOperator(Operators &anOp);
  StatusResult parseAndMergeOperator(Operators &anOp);
  StatusResult parseOperand(const Entity &anEntity, Operand &anOperand);

 protected:
  Tokenizer &tokenizer;
};

}  // namespace ECE141

#endif /* ParseHelper_hpp */
