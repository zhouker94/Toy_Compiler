#pragma once

#include "spdlog/spdlog.h"

#include "ast.hpp"
#include "lexer.hpp"

namespace minicc {
// Parser: consumes a token stream and produces AST.
class Parser {
public:
  Parser() = default;
  // parse tokens and return a single program node
  std::unique_ptr<ProgramNode> parse(const std::vector<Token> &tokens);

private:
  // State
  const std::vector<Token> *m_tokens;
  size_t m_pos = 0;

  // Helper methods
  const Token &peek(size_t offset = 0) const;
  const Token &consume();
  const Token &consume(TOKEN_TYPE type, const std::string &message);
  bool is_at_end() const;
  [[noreturn]] void error(const Token &token, const std::string &message);
  int get_prec(TOKEN_TYPE type);

  // Parsing methods for grammar rules
  std::unique_ptr<FuncDefNode> parse_function_definition();

  std::unique_ptr<ASTNode> parse_statement();
  std::unique_ptr<BlockStmtNode> parse_block_statement();
  std::unique_ptr<VarDeclNode> parse_var_decl_statement();
  std::unique_ptr<ReturnStmtNode> parse_return_statement();
  std::unique_ptr<IfStmtNode> parse_if_statement();
  std::unique_ptr<WhileStmtNode> parse_while_statement();

  std::unique_ptr<ASTNode> parse_expression();
  std::unique_ptr<ASTNode> parse_primary_expression();
  std::unique_ptr<ASTNode> parse_binary_expression(int minPrec = 0);
  std::unique_ptr<ASTNode> parse_assignment_expression();
};

} // namespace minicc
