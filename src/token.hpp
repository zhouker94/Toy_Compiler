#pragma once

#include <string>
#include <unordered_map>

namespace minicc {

enum TOKEN_TYPE {
  // Keywords
  T_INT,
  T_VOID,
  T_IF,
  T_ELSE,
  T_WHILE,
  T_RETURN,

  T_IDENTIFIER,
  T_NUMBER,

  // Operators
  T_PLUS,
  T_MINUS,
  T_STAR,
  T_SLASH,
  T_EQ,
  T_NEQ,
  T_LT,
  T_GT,
  T_LTE,
  T_GTE,
  T_ASSIGN,

  // Punctuation
  T_LPAREN,
  T_RPAREN,
  T_LBRACE,
  T_RBRACE,
  T_SEMICOLON,
  T_COMMA,

  // Misc
  T_UNKNOWN,
  T_EOF,
};

struct Token {
  TOKEN_TYPE type;
  std::string text;
  size_t line = 0;
  size_t column = 0;
};

TOKEN_TYPE check_token_type(const std::string &);

const char *token_type_to_string(TOKEN_TYPE t);

} // namespace minicc
