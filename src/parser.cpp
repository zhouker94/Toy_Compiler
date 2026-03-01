#include <stdexcept>

#include "spdlog/spdlog.h"

#include "ast.hpp"
#include "parser.hpp"
#include "utils.hpp"

namespace minicc {
// Main parse entry point
std::unique_ptr<ProgramNode> Parser::parse(const std::vector<Token> &tokens) {
  m_tokens = &tokens;
  m_pos = 0; // Reset position for each parse call

  auto program_node = std::make_unique<ProgramNode>();

  while (!is_at_end()) {
    try {
      // Lookahead to distinguish between global variable and function
      // Both start with type + identifier. If followed by '(', it's a function.
      if (peek(2).type == T_LPAREN) {
        program_node->declarations.push_back(parse_function_definition());
      } else {
        program_node->declarations.push_back(parse_var_decl_statement());
      }
    } catch (const std::runtime_error &e) {
      spdlog::get("parser_logger")->error("Parsing error: {}", e.what());
      // In a real compiler, we might try to synchronize and continue.
      // For now, we'll just stop.
      break;
    }
  }

  return program_node;
}

// Helper method implementations
const Token &Parser::peek(size_t offset) const {
  if (m_pos + offset >= m_tokens->size()) {
    return m_tokens->back();
  }
  return (*m_tokens)[m_pos + offset];
}

const Token &Parser::consume() {
  if (!is_at_end()) {
    m_pos++;
  }
  return (*m_tokens)[m_pos - 1];
}

const Token &Parser::consume(TOKEN_TYPE type, const std::string &message) {
  if (peek().type == type) {
    return consume();
  }
  error(peek(), message);
  // This calls the Parser::error method, which now uses m_logger
  // This part will not be reached because error() throws.
  // It's here to satisfy the return type requirement.
  throw std::runtime_error("Unexpected error flow in consume");
}

void Parser::error(const Token &token, const std::string &message) {
  std::string error_message;
  if (token.type == T_EOF) {
    error_message = fmt::format("Error at end of file: {}", message);
  } else {
    error_message = fmt::format("Error at line {}, column {}: {} (near '{}')",
                                token.line, token.column, message, token.text);
  }
  spdlog::get("parser_logger")->error(error_message);
  throw std::runtime_error(error_message);
}

bool Parser::is_at_end() const {
  return m_pos >= m_tokens->size() || peek().type == T_EOF;
}

std::unique_ptr<FuncDefNode> Parser::parse_function_definition() {
  TOKEN_TYPE return_type;
  if (peek().type == T_INT || peek().type == T_VOID) {
    return_type = consume().type;
  } else {
    error(peek(), "Expected 'int' or 'void' for function return type.");
  }

  const Token &name_token = consume(T_IDENTIFIER, "Expected function name.");

  consume(T_LPAREN, "Expected '(' after function name.");
  std::vector<std::unique_ptr<VarDeclNode>> params;
  if (peek().type != T_RPAREN) {
    do {
      consume(T_INT, "Expected 'int' for parameter type.");
      std::string p_name =
          consume(T_IDENTIFIER, "Expected parameter name.").text;
      params.push_back(std::make_unique<VarDeclNode>(std::move(p_name)));
      if (peek().type == T_COMMA)
        consume();
      else
        break;
    } while (true);
  }
  consume(T_RPAREN, "Expected ')' after parameters.");

  return std::make_unique<FuncDefNode>(
      return_type, name_token.text, std::move(params), parse_block_statement());
}

std::unique_ptr<BlockStmtNode> Parser::parse_block_statement() {
  auto block_node = std::make_unique<BlockStmtNode>();
  consume(T_LBRACE, "Expected '{' to start a block.");

  while (peek().type != T_RBRACE && !is_at_end()) {
    block_node->statements.push_back(parse_statement());
  }

  consume(T_RBRACE, "Expected '}' to end a block.");
  return block_node;
}

std::unique_ptr<ASTNode> Parser::parse_statement() {
  if (peek().type == T_RETURN) {
    return parse_return_statement();
  }

  if (peek().type == T_INT) {
    return parse_var_decl_statement();
  }

  if (peek().type == T_IF) {
    return parse_if_statement();
  }

  if (peek().type == T_WHILE) {
    return parse_while_statement();
  }

  if (peek().type == T_LBRACE) {
    return parse_block_statement();
  }

  if (peek().type == T_SEMICOLON) {
    consume();
    return std::make_unique<EmptyStmtNode>();
  }

  // If it's not a return or var declaration, it must be an expression statement
  auto expr = parse_expression();
  consume(T_SEMICOLON, "Expected ';' after an expression.");
  return std::make_unique<ExprStmtNode>(std::move(expr));
}

std::unique_ptr<IfStmtNode> Parser::parse_if_statement() {
  consume();
  consume(T_LPAREN, "Expected '(' after if keyword.");
  auto cond = parse_expression();
  consume(T_RPAREN, "Expected ')' after if condition.");
  std::unique_ptr<ASTNode> then_br = parse_statement();

  std::unique_ptr<ASTNode> else_br = nullptr;
  if (peek().type == T_ELSE) {
    consume();
    else_br = parse_statement();
  }
  auto if_node = std::make_unique<IfStmtNode>(
      std::move(cond), std::move(then_br), std::move(else_br));
  return if_node;
}

std::unique_ptr<WhileStmtNode> Parser::parse_while_statement() {
  consume(); // Consume 'while'
  consume(T_LPAREN, "Expected '(' after while keyword.");
  auto cond = parse_expression();
  consume(T_RPAREN, "Expected ')' after while condition.");
  auto body = parse_statement();

  return std::make_unique<WhileStmtNode>(std::move(cond), std::move(body));
}

std::unique_ptr<VarDeclNode> Parser::parse_var_decl_statement() {
  consume(T_INT, "Expected 'int' to start a variable declaration.");
  std::string name = consume(T_IDENTIFIER, "Expected variable name.").text;

  std::unique_ptr<ASTNode> initializer = nullptr;
  if (peek().type == T_ASSIGN) {
    consume();
    initializer = parse_expression();
  }

  consume(T_SEMICOLON, "Expected ';' after variable declaration.");
  return std::make_unique<VarDeclNode>(std::move(name), std::move(initializer));
}

std::unique_ptr<ReturnStmtNode> Parser::parse_return_statement() {
  consume(); // Consume 'return'
  auto expr = parse_expression();
  consume(T_SEMICOLON, "Expected ';' after return value.");

  auto node = std::make_unique<ReturnStmtNode>(std::move(expr));
  return node;
}

std::unique_ptr<ASTNode> Parser::parse_expression() {
  return parse_assignment_expression();
}

int Parser::get_prec(TOKEN_TYPE type) {
  switch (type) {
  case T_STAR:
  case T_SLASH:
    return 5; // Multiplicative operators
  case T_PLUS:
  case T_MINUS:
    return 4; // Additive operators
  case T_LT:
  case T_GT:
  case T_LTE:
  case T_GTE:
    return 3; // Relational operators (same precedence as equality for
              // simplicity)
  case T_EQ:
  case T_NEQ:
    return 2; // Equality operators
  default:
    return -1; // Default for non-operator tokens
  }
}

std::unique_ptr<ASTNode> Parser::parse_binary_expression(int minPrec) {
  auto left = parse_primary_expression();

  while (get_prec(peek().type) >= minPrec) {
    Token op = consume();
    int nextMinPrec = get_prec(op.type) + 1;
    auto right = parse_binary_expression(nextMinPrec);
    left =
        std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
  }

  return left;
}

std::unique_ptr<ASTNode> Parser::parse_primary_expression() {
  if (peek().type == T_NUMBER) {
    const Token &token = consume();
    long long value;
    try {
      value = std::stoll(token.text);
    } catch (const std::exception &e) {
      error(token, "Invalid integer literal.");
    }
    return std::make_unique<IntLiteralNode>(value);
  }

  if (peek().type == T_IDENTIFIER) {
    const Token &token = consume();
    // Check for function call
    if (peek().type == T_LPAREN) {
      consume(); // consume '('
      std::vector<std::unique_ptr<ASTNode>> args;
      if (peek().type != T_RPAREN) {
        do {
          args.push_back(parse_expression());
          if (peek().type == T_COMMA)
            consume();
          else
            break;
        } while (true);
      }
      consume(T_RPAREN, "Expected ')' after arguments.");
      return std::make_unique<CallExprNode>(token.text, std::move(args));
    }
    return std::make_unique<IdentifierNode>(token.text);
  }

  if (peek().type == T_LPAREN) {
    consume();
    auto node = parse_expression();
    consume(T_RPAREN, "Expected a ')'");
    return node;
  }

  error(peek(), "Expected an expression (e.g., a number).");
  return nullptr; // Unreachable
}

std::unique_ptr<ASTNode> Parser::parse_assignment_expression() {
  // Parse the left-hand side, which can be a full binary expression
  // Start parse_binary_expression with minPrec 0 to allow it to parse all
  // binary ops *except* assignment
  auto left = parse_binary_expression(0);

  if (peek().type == T_ASSIGN) {
    Token assign_op = consume(); // Consume the '=' token

    // Check if the left-hand side is a valid target for assignment
    if (left->kind != NodeKind::NK_IDENTIFIER) {
      error(
          assign_op,
          "Invalid assignment target: expected an identifier."); // Uses
                                                                 // Parser::error
    }

    // Recursively parse the right-hand side for right-associativity
    auto right = parse_assignment_expression();

    auto assign_node =
        std::make_unique<AssignExprNode>(std::move(left), std::move(right));
    return assign_node;
  }

  return left; // If no assignment operator, return the parsed expression
}
} // namespace minicc
