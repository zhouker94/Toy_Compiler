#include "token.hpp"

namespace minicc {

TOKEN_TYPE check_token_type(const std::string &str) {
  // classify a single token produced by tokenize
  if (str == "int")
    return T_INT;
  if (str == "void")
    return T_VOID;
  if (str == "if")
    return T_IF;
  if (str == "else")
    return T_ELSE;
  if (str == "while")
    return T_WHILE;
  if (str == "return")
    return T_RETURN;

  if (str == "+")
    return T_PLUS;
  if (str == "-")
    return T_MINUS;
  if (str == "*")
    return T_STAR;
  if (str == "/")
    return T_SLASH;
  if (str == "==")
    return T_EQ;
  if (str == "!=")
    return T_NEQ;
  if (str == "<")
    return T_LT;
  if (str == ">")
    return T_GT;
  if (str == "<=")
    return T_LTE;
  if (str == ">=")
    return T_GTE;
  if (str == "=")
    return T_ASSIGN;

  if (str == "(")
    return T_LPAREN;
  if (str == ")")
    return T_RPAREN;
  if (str == "{")
    return T_LBRACE;
  if (str == "}")
    return T_RBRACE;
  if (str == ";")
    return T_SEMICOLON;
  if (str == ",")
    return T_COMMA;

  // number: starts with digit
  if (std::isdigit(static_cast<unsigned char>(str[0])))
    return T_NUMBER;

  // identifier: begins with letter or underscore and isn't a keyword
  if (std::isalpha(static_cast<unsigned char>(str[0])) || str[0] == '_')
    return T_IDENTIFIER;

  // default fallback
  return T_UNKNOWN;
}

static const std::unordered_map<TOKEN_TYPE, const char *> token_map = {
    {T_INT, "KW_INT"},
    {T_VOID, "KW_VOID"},
    {T_IF, "KW_IF"},
    {T_ELSE, "KW_ELSE"},
    {T_WHILE, "KW_WHILE"},
    {T_RETURN, "KW_RETURN"},
    {T_IDENTIFIER, "IDENT"},
    {T_NUMBER, "NUMBER"},
    {T_PLUS, "PLUS"},
    {T_MINUS, "MINUS"},
    {T_STAR, "STAR"},
    {T_SLASH, "SLASH"},
    {T_EQ, "EQ"},
    {T_NEQ, "NEQ"},
    {T_LT, "LT"},
    {T_GT, "GT"},
    {T_LTE, "LTE"},
    {T_GTE, "GTE"},
    {T_ASSIGN, "ASSIGN"},
    {T_LPAREN, "LPAREN"},
    {T_RPAREN, "RPAREN"},
    {T_LBRACE, "LBRACE"},
    {T_RBRACE, "RBRACE"},
    {T_SEMICOLON, "SEMICOLON"},
    {T_COMMA, "COMMA"},
    {T_UNKNOWN, "UNKNOWN"},
    {T_EOF, "EOF"},
};

const char *token_type_to_string(TOKEN_TYPE t) {
  auto it = token_map.find(t);
  if (it == token_map.end()) {
    return "UNK";
  }
  return it->second;
}

} // namespace minicc
