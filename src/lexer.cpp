#include <algorithm>
#include <format>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <stdexcept>

#include "lexer.hpp"
#include "utils.hpp"

#include "spdlog/fmt/fmt.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace minicc {

std::vector<Token> Lexer::tokenize(const std::string &source) {
  std::vector<Token> tokens;
  size_t i = 0, n = source.size();
  size_t line = 1;
  size_t line_start = 0;

  while (i < n) {
    char c = source[i];
    size_t column = i - line_start + 1;

    // skip whitespace
    if (std::isspace(static_cast<unsigned char>(c))) {
      if (c == '\n') {
        line++;
        line_start = i + 1;
      }
      ++i;
      continue;
    }

    // identifier or keyword
    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
      size_t start = i;
      ++i;
      while (i < n && (std::isalnum(static_cast<unsigned char>(source[i])) ||
                       source[i] == '_'))
        ++i;
      {
        std::string txt = source.substr(start, i - start);
        tokens.push_back({check_token_type(txt), std::move(txt), line, column});
      }
      continue;
    }

    // number literal
    if (std::isdigit(static_cast<unsigned char>(c))) {
      size_t start = i;
      while (i < n && std::isdigit(static_cast<unsigned char>(source[i])))
        ++i;

      {
        std::string txt = source.substr(start, i - start);
        tokens.push_back({check_token_type(txt), std::move(txt), line, column});
      }
      continue;
    }

    // comments
    if (c == '/' && i + 1 < n) {
      if (source[i + 1] == '/') {
        // rest of line is a comment, skip
        i += 2;
        while (i < n && source[i] != '\n') {
          ++i;
        }
        continue;
      }
      if (source[i + 1] == '*') {
        // multi-line comment, skip
        size_t comment_line = line;
        size_t comment_column = column;
        i += 2;
        while (i + 1 < n && !(source[i] == '*' && source[i + 1] == '/')) {
          if (source[i] == '\n') {
            line++;
            line_start = i + 1;
          }
          ++i;
        }
        if (i + 1 >= n) {
          throw std::runtime_error(
              fmt::format("Lexer Error: Unterminated block comment starting at "
                          "line {}, column {}",
                          comment_line, comment_column));
        }
        i += 2; // consume */
        continue;
      }
    }

    // operators and punctuation
    std::string op_str;
    if (i + 1 < n) {
      op_str = source.substr(i, 2);
      // check for two-char operators
      if (op_str == "==" || op_str == "!=" || op_str == "<=" ||
          op_str == ">=") {
        tokens.push_back({check_token_type(op_str), op_str, line, column});
        i += 2;
        continue;
      }
    }
    // fall back to one-char tokens
    op_str = source.substr(i, 1);
    TOKEN_TYPE type = check_token_type(op_str);
    if (type == T_UNKNOWN) {
      throw std::runtime_error(fmt::format(
          "Lexer Error: Unrecognized character '{}' at line {}, column {}",
          op_str, line, column));
    }
    tokens.push_back({type, op_str, line, column});
    ++i;
  }

  tokens.push_back({T_EOF, "", line, i - line_start + 1});
  print_tokens(source, tokens);
  return tokens;
}

void Lexer::print_tokens(const std::string &source,
                         const std::vector<Token> &tokens) {
  auto logger = spdlog::get("lexer_logger");
  if (!logger || !logger->should_log(spdlog::level::info))
    return;

  // Split source into lines for visualization
  std::vector<std::string> lines;
  std::string segment;
  std::stringstream ss(source);
  while (std::getline(ss, segment)) {
    lines.push_back(segment);
  }

  logger->info("=== Visual Token Map ===");

  size_t token_idx = 0;
  for (size_t l = 0; l < lines.size(); ++l) {
    // Collect all tokens belonging to this line
    std::vector<const Token *> line_tokens;
    while (token_idx < tokens.size() && tokens[token_idx].line == l + 1) {
      if (tokens[token_idx].type != T_EOF) {
        line_tokens.push_back(&tokens[token_idx]);
      }
      token_idx++;
    }

    if (line_tokens.empty())
      continue;

    // 1. Print the original source line
    logger->info("{:>3} | {}", l + 1, lines[l]);

    // 2. Build and print the mapping line
    // We use two lines: one for carets '^' and one for the type names
    std::string carets = "    | ";
    std::string types = "    | ";
    size_t current_pos = 1;

    for (const auto *t : line_tokens) {
      // Move to the token's column
      while (current_pos < t->column) {
        carets += " ";
        current_pos++;
      }

      // Ensure 'types' line catches up to the current caret position
      if (types.length() < carets.length()) {
        types.append(carets.length() - types.length(), ' ');
      }

      // If the previous label is long and overlaps the current caret,
      // add a single space to prevent them from sticking together.
      if (types.length() > carets.length()) {
        types += " ";
      }

      carets += "^";
      types += "[";
      types += token_type_to_string(t->type);
      types += "]";
      current_pos++;
    }

    logger->info("{}", carets);
    logger->info("{}", types);
  }

  logger->info("========================");
}
} // namespace minicc
