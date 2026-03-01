#pragma once

#include <iostream>
#include <set>
#include <stdio.h>
#include <string>
#include <vector>

#include "token.hpp"

namespace minicc {

// keywords for a subset of C
extern const std::set<std::string> KEYWORD_LIST;

class Lexer {
private:
  static void print_tokens(const std::string &source,
                           const std::vector<Token> &tokens);

public:
  static std::vector<Token> tokenize(const std::string &source);
};

} // namespace minicc
