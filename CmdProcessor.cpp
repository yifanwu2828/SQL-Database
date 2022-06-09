/**
 * @file CmdProcessor.cpp
 * @author Yifan Wu
 * @brief 
 * @version 0.1
 * @date 2022-03-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "CmdProcessor.hpp"

#include <utility>

namespace ECE141 {

CmdProcessor::CmdProcessor(std::ostream &anOutput, std::string aName)
    : output{anOutput}, name{std::move(aName)} {}

}  // namespace ECE141
