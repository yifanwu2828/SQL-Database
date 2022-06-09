/**
 * @file Helpers.hpp
 * @author Yifan Wu
 * @brief 
 * @version 0.9
 * @date 2022-06-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef Helpers_h
#define Helpers_h

#include <algorithm>
#include <array>
#include <cctype>
#include <functional>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "BasicTypes.hpp"
#include "Errors.hpp"
#include "keywords.hpp"

namespace ECE141 {

template <typename T, size_t aSize>
bool in_array(T (&anArray)[aSize], const T &aValue) {
  return std::find(std::begin(anArray), std::end(anArray), aValue) !=
         std::end(anArray);
}

template <typename T, size_t aSize>
bool in_array(const std::array<T, aSize> &anArray, const T &aValue) {
  return std::find(anArray.begin(), anArray.end(), aValue) != anArray.end();
}

// ? custom --------------------------------------------------------------------
static const std::map<Logical, std::string> kwToLogStrMap = {
    {Logical::and_op, "AND"},
    {Logical::or_op, "OR"},
    {Logical::not_op, "NOT"},
    {Logical::no_op, ""},
    {Logical::unknown_op, "unknown"}};

static const std::map<Operators, std::string> ExpressionOpsToStr{
    {
        Operators::equal_op,
        "=",
    },
    {
        Operators::lt_op,
        "<",
    },
    {
        Operators::lte_op,
        "<=",
    },
    {
        Operators::gt_op,
        ">",
    },
    {
        Operators::gte_op,
        ">=",
    },
    {
        Operators::notequal_op,
        "!=",
    },
};

static const std::map<std::string, Operators> gExpressionOps{
    {"=", Operators::equal_op},      {"<", Operators::lt_op},
    {"<=", Operators::lte_op},       {">", Operators::gt_op},
    {">=", Operators::gte_op},       {"!=", Operators::notequal_op},
    {"not", Operators::notequal_op},
};

static std::map<std::string, Operators> gOperators{
    std::make_pair(".", Operators::dot_op),
    std::make_pair("+", Operators::add_op),
    std::make_pair("-", Operators::subtract_op),
    std::make_pair("*", Operators::multiply_op),
    std::make_pair("/", Operators::divide_op),
    std::make_pair("^", Operators::power_op),
    std::make_pair("%", Operators::mod_op),
    std::make_pair("=", Operators::equal_op),
    std::make_pair("!=", Operators::notequal_op),
    std::make_pair("not", Operators::notequal_op),
    std::make_pair("<", Operators::lt_op),
    std::make_pair("<=", Operators::lte_op),
    std::make_pair(">", Operators::gt_op),
    std::make_pair(">=", Operators::gte_op),
    std::make_pair("and", Operators::and_op),
    std::make_pair("or", Operators::or_op),
    std::make_pair("nor", Operators::nor_op),
    std::make_pair("between", Operators::between_op),

    // custom ---------------------------------------------------------
    std::make_pair("!", Operators::not_op)};

const size_t gJoinTypesNum = 5;
static const std::array<ECE141::Keywords, gJoinTypesNum> gJoinTypes{
    ECE141::Keywords::cross_kw, ECE141::Keywords::full_kw,
    ECE141::Keywords::inner_kw, ECE141::Keywords::left_kw,
    ECE141::Keywords::right_kw};

// a list of known functions...
static const std::map<std::string, int> gFunctions{
    std::make_pair("avg", 10), std::make_pair("count", 20),
    std::make_pair("max", 30), std::make_pair("min", 40)};

// This map binds a keyword string with a Keyword (token)...
static const std::map<std::string, Keywords> gDictionary{
    std::make_pair("add", Keywords::add_kw),
    std::make_pair("all", Keywords::all_kw),
    std::make_pair("alter", Keywords::alter_kw),
    std::make_pair("and", Keywords::and_kw),
    std::make_pair("as", Keywords::as_kw),
    std::make_pair("asc", Keywords::asc_kw),
    std::make_pair("avg", ECE141::Keywords::avg_kw),
    std::make_pair("auto_increment", Keywords::auto_increment_kw),
    std::make_pair("between", ECE141::Keywords::between_kw),
    std::make_pair("boolean", ECE141::Keywords::boolean_kw),
    std::make_pair("by", ECE141::Keywords::by_kw),
    std::make_pair("change", ECE141::Keywords::change_kw),
    std::make_pair("changed", ECE141::Keywords::changed_kw),
    std::make_pair("char", ECE141::Keywords::char_kw),
    std::make_pair("column", ECE141::Keywords::column_kw),
    std::make_pair("count", ECE141::Keywords::count_kw),
    std::make_pair("create", ECE141::Keywords::create_kw),
    std::make_pair("cross", ECE141::Keywords::cross_kw),
    std::make_pair("current_date", Keywords::current_date_kw),
    std::make_pair("current_time", Keywords::current_time_kw),
    std::make_pair("current_timestamp", Keywords::current_timestamp_kw),
    std::make_pair("database", ECE141::Keywords::database_kw),
    std::make_pair("databases", ECE141::Keywords::databases_kw),
    std::make_pair("datetime", ECE141::Keywords::datetime_kw),
    std::make_pair("decimal", ECE141::Keywords::decimal_kw),
    std::make_pair("desc", ECE141::Keywords::desc_kw),
    std::make_pair("delete", ECE141::Keywords::delete_kw),
    std::make_pair("describe", ECE141::Keywords::describe_kw),
    std::make_pair("distinct", ECE141::Keywords::distinct_kw),
    std::make_pair("double", ECE141::Keywords::double_kw),
    std::make_pair("drop", ECE141::Keywords::drop_kw),
    std::make_pair("dump", ECE141::Keywords::dump_kw),
    std::make_pair("enum", ECE141::Keywords::enum_kw),
    std::make_pair("explain", ECE141::Keywords::explain_kw),
    std::make_pair("false", ECE141::Keywords::false_kw),
    std::make_pair("float", ECE141::Keywords::float_kw),
    std::make_pair("foreign", ECE141::Keywords::foreign_kw),
    std::make_pair("from", ECE141::Keywords::from_kw),
    std::make_pair("full", ECE141::Keywords::full_kw),
    std::make_pair("group", ECE141::Keywords::group_kw),
    std::make_pair("help", ECE141::Keywords::help_kw),
    std::make_pair("in", ECE141::Keywords::in_kw),
    std::make_pair("index", ECE141::Keywords::index_kw),
    std::make_pair("indexes", ECE141::Keywords::indexes_kw),
    std::make_pair("inner", ECE141::Keywords::inner_kw),
    std::make_pair("insert", ECE141::Keywords::insert_kw),
    std::make_pair("int", ECE141::Keywords::integer_kw),
    std::make_pair("integer", ECE141::Keywords::integer_kw),
    std::make_pair("into", ECE141::Keywords::into_kw),
    std::make_pair("join", ECE141::Keywords::join_kw),
    std::make_pair("key", ECE141::Keywords::key_kw),
    std::make_pair("last", ECE141::Keywords::last_kw),
    std::make_pair("left", ECE141::Keywords::left_kw),
    std::make_pair("like", ECE141::Keywords::like_kw),
    std::make_pair("limit", ECE141::Keywords::limit_kw),
    std::make_pair("max", ECE141::Keywords::max_kw),
    std::make_pair("min", ECE141::Keywords::min_kw),
    std::make_pair("modify", ECE141::Keywords::modify_kw),
    std::make_pair("not", ECE141::Keywords::not_kw),
    std::make_pair("null", ECE141::Keywords::null_kw),
    std::make_pair("on", ECE141::Keywords::on_kw),
    std::make_pair("or", ECE141::Keywords::or_kw),
    std::make_pair("order", ECE141::Keywords::order_kw),
    std::make_pair("outer", ECE141::Keywords::outer_kw),
    std::make_pair("primary", ECE141::Keywords::primary_kw),
    std::make_pair("query", ECE141::Keywords::query_kw),
    std::make_pair("quit", ECE141::Keywords::quit_kw),
    std::make_pair("references", ECE141::Keywords::references_kw),
    std::make_pair("right", ECE141::Keywords::right_kw),
    std::make_pair("rows", ECE141::Keywords::rows_kw),
    std::make_pair("select", ECE141::Keywords::select_kw),
    std::make_pair("self", ECE141::Keywords::self_kw),
    std::make_pair("set", ECE141::Keywords::set_kw),
    std::make_pair("show", ECE141::Keywords::show_kw),
    std::make_pair("sum", ECE141::Keywords::sum_kw),
    std::make_pair("table", ECE141::Keywords::table_kw),
    std::make_pair("tables", ECE141::Keywords::tables_kw),
    std::make_pair("true", ECE141::Keywords::true_kw),
    std::make_pair("unique", ECE141::Keywords::unique_kw),
    std::make_pair("update", ECE141::Keywords::update_kw),
    std::make_pair("use", ECE141::Keywords::use_kw),
    std::make_pair("values", ECE141::Keywords::values_kw),
    std::make_pair("varchar", ECE141::Keywords::varchar_kw),
    std::make_pair("version", ECE141::Keywords::version_kw),
    std::make_pair("where", ECE141::Keywords::where_kw),

    // custom ---------------------------------------------------------
    std::make_pair("default", ECE141::Keywords::default_kw),
    std::make_pair("run", ECE141::Keywords::run_kw),
};

static const std::map<Keywords, DataTypes> gKeywordTypes{
    std::make_pair(Keywords::boolean_kw, DataTypes::bool_type),
    std::make_pair(Keywords::datetime_kw, DataTypes::datetime_type),
    std::make_pair(Keywords::float_kw, DataTypes::float_type),
    std::make_pair(Keywords::integer_kw, DataTypes::int_type),
    std::make_pair(Keywords::varchar_kw, DataTypes::varchar_type)};

class Helpers {
 public:
  // be very careful about collisions!
  template <typename T>
  static constexpr uint32_t hashString(T str) {
    std::size_t str_hash = std::hash<T>{}(str);
    return str_hash;
    // auto s = str.c_str();
    // uint32_t h{0};
    // unsigned char *p;
    // const int gMultiplier{37};
    // for (p = (unsigned char*)str; *p != '\0'; p++)
    //   h = gMultiplier * h + *p;
    // return h;
  }

  static Keywords getKeywordId(const std::string &aKeyword) {
    auto theIter = gDictionary.find(aKeyword);
    if (theIter != gDictionary.end()) {
      return theIter->second;
    }
    return Keywords::unknown_kw;
  }

  // convert from char to keyword...
  static constexpr Keywords charToKeyword(char aChar) {
    switch (toupper(aChar)) {
      case 'I':
        return Keywords::integer_kw;
      case 'T':
        return Keywords::datetime_kw;
      case 'B':
        return Keywords::boolean_kw;
      case 'F':
        return Keywords::float_kw;
      case 'V':
        return Keywords::varchar_kw;
      default:
        return Keywords::unknown_kw;
    }
  }

  static const char *dataTypeToString(DataTypes aType) {
    switch (aType) {
      case DataTypes::no_type:
        return "none";
      case DataTypes::bool_type:
        return "bool";
      case DataTypes::datetime_type:
        return "datetime";
      case DataTypes::float_type:
        return "float";
      case DataTypes::int_type:
        return "int";
      case DataTypes::varchar_type:
        return "varchar";
      default:
        return nullptr;
    }
  }

  static const char *keywordToString(Keywords aType) {
    switch (aType) {
      case Keywords::boolean_kw:
        return "bool";
      case Keywords::create_kw:
        return "create";
      case Keywords::database_kw:
        return "database";
      case Keywords::databases_kw:
        return "databases";
      case Keywords::datetime_kw:
        return "datetime";
      case Keywords::describe_kw:
        return "describe";
      case Keywords::drop_kw:
        return "drop";
      case Keywords::float_kw:
        return "float";
      case Keywords::integer_kw:
        return "integer";
      case Keywords::show_kw:
        return "show";
      case Keywords::table_kw:
        return "table";
      case Keywords::tables_kw:
        return "tables";
      case Keywords::use_kw:
        return "use";
      case Keywords::varchar_kw:
        return "varchar";
      default:
        return "unknown";
    }
  }

  // USE: ---validate that given keyword is a datatype...
  static constexpr bool isDatatype(Keywords aKeyword) {
    switch (aKeyword) {
      case Keywords::char_kw:
      case Keywords::datetime_kw:
      case Keywords::float_kw:
      case Keywords::integer_kw:
      case Keywords::varchar_kw:
        return true;
      default:
        return false;
    }
  }

  static DataTypes getTypeForKeyword(Keywords aKeyword) {
    return gKeywordTypes.find(aKeyword) != gKeywordTypes.end()
               ? gKeywordTypes.at(aKeyword)
               : DataTypes::no_type;
  }

  static Operators toOperator(const std::string &aString) {
    auto theIter = gOperators.find(aString);
    if (theIter != gOperators.end()) {
      return theIter->second;
    }
    return Operators::unknown_op;
  }

  static int getFunctionId(const std::string &anIdentifier) {
    auto theIter = gFunctions.find(anIdentifier);
    if (theIter != gFunctions.end()) {
      return theIter->second;
    }
    return 0;
  }

  static bool isNumericKeyword(Keywords aKeyword) {
    static std::array<Keywords, 4> theTypes{
        Keywords::decimal_kw, Keywords::double_kw, Keywords::float_kw,
        Keywords::integer_kw};

    return std::all_of(theTypes.begin(), theTypes.end(),
                       [&](Keywords k) { return aKeyword == k; });
  }

  static std::string QueryOk(size_t aCount, double anElapsed) {
    return "Query Ok, " + std::to_string(aCount) + " rows affected (" +
           std::to_string(anElapsed) + ") secs)\n";
  }

  // ? custom
  // --------------------------------------------------------------------
  static std::string valToString(const Value &aVal) {
    // using Value = std::variant<bool, int, double, std::string>;
    const size_t BOOL = 0;
    const size_t INTEGER = 1;
    const size_t DOUBLE = 2;
    const size_t STRING = 3;
    std::string theValue;
    std::string theType;

    switch (aVal.index()) {
      case (BOOL):
        theType = static_cast<char>(DataTypes::bool_type);
        theValue = std::get<bool>(aVal) ? "true" : "false";
        break;
      case (INTEGER):
        theType = static_cast<char>(DataTypes::int_type);
        theValue = std::to_string(std::get<int>(aVal));
        break;
      case (DOUBLE):
        theType = static_cast<char>(DataTypes::float_type);
        theValue = std::to_string(std::get<double>(aVal));
        break;
      case (STRING):
        theType = static_cast<char>(DataTypes::varchar_type);
        theValue = std::get<std::string>(aVal);
        break;
      default:
        theType = static_cast<char>(DataTypes::no_type);
        theValue = "NULL";
    }
    return theValue + theType;
  }

  // ? custom
  // --------------------------------------------------------------------
  static Value toValue(const std::string &aValStr, DataTypes aDType) {
    Value theVal;

    if (DataTypes::int_type == aDType || DataTypes::datetime_type == aDType) {
      theVal = std::stoi(aValStr);
    } else if (DataTypes::float_type == aDType) {
      theVal = std::stod(aValStr);
    } else if (DataTypes::bool_type == aDType) {
      theVal = aValStr == "true";
    } else if (DataTypes::varchar_type == aDType) {
      theVal = aValStr;
    } else {
      // else if (DataTypes::no_type == aDType)
      theVal = "NULL";
    }
    return theVal;
  }

  // ? custom
  // --------------------------------------------------------------------
  static Logical kwToLogOp(Keywords aKW) {
    static std::map<Keywords, Logical> kwLogOpMap = {
        {Keywords::and_kw, Logical::and_op},
        {Keywords::or_kw, Logical::or_op},
        {Keywords::not_kw, Logical::not_op}};

    if (kwLogOpMap.find(aKW) != kwLogOpMap.end()) {
      return kwLogOpMap.at(aKW);
    }
    return Logical::no_op;
  }

  // ? custom
  // --------------------------------------------------------------------
  static Operators oppositeOpOf(Operators anOp) {
    static std::map<Operators, Operators> operatorInverses = {
        {Operators::lt_op, Operators::gte_op},
        {Operators::gt_op, Operators::lte_op},
        {Operators::lte_op, Operators::gt_op},
        {Operators::gte_op, Operators::lt_op},
        {Operators::equal_op, Operators::notequal_op},
        {Operators::notequal_op, Operators::equal_op}};

    if (operatorInverses.find(anOp) != operatorInverses.end()) {
      return operatorInverses[anOp];
    }
    return Operators::unknown_op;
  }

  static std::string replaceWith(std::string aStr, const char &src,
                                 const char &dest) {
    const std::regex target{src};
    const std::string replacement{dest};
    return std::regex_replace(aStr, target, replacement);
  }

  static std::string replaceWith(std::string aStr, const std::string &src,
                                 const std::string &dest) {
    const std::regex target{src};
    const std::string replacement{dest};
    return std::regex_replace(aStr, target, replacement);
  }

  static std::streamsize getWidthFromVal(const Value &aVal) {
    // using Value = std::variant<bool, int, double, std::string>;
    const size_t BOOL = 0;
    const size_t INTEGER = 1;
    const size_t DOUBLE = 2;
    const size_t STRING = 3;
    // std::string theValue;
    // std::string theType;
    std::streamsize theWidth;
    switch (aVal.index()) {
      case (BOOL):
        theWidth = 6;
        break;
      case (INTEGER):
        // INT_MAX: 2147483647
        theWidth = 8;
        break;
      case (DOUBLE):
        theWidth = 10;
        break;
      case (STRING):
        theWidth = 20;
        break;
      default:
        theWidth = 20;
    }
    return theWidth;
  }

  static StatusResult checkStreamNotFail(std::istream &anInput) {
    if (anInput.fail()) {
      std::cerr << "EOF\n";
      return {Errors::eofError};
    }
    return {Errors::noError};
  }

  // ? custom
  // --------------------------------------------------------------------
  template <typename Numeric>
  static bool isNumber(const std::string &aStr) {
    Numeric n;
    return ((std::istringstream(aStr) >> n >> std::ws).eof());
  }

  template <typename T>
  static StatusResult encodeInto(std::ostream &anOutput, const T &anObj) {
    anOutput << anObj << ' ';
    return {Errors::noError};
  }

  template <typename T>
  static StatusResult decodeFrom(std::istream &anInput, T &anObj) {
    anInput >> anObj;
    return checkStreamNotFail(anInput);
  }
};
}  // namespace ECE141

#endif /* Helpers_h */
