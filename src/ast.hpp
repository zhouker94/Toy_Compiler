#pragma once

#include "spdlog/spdlog.h"

#include "lexer.hpp"

namespace minicc {

// AST Node Kinds for the simple C subset
enum class NodeKind {
  // Program Root
  NK_PROGRAM,

  // Statements
  NK_RETURN_STMT,
  NK_IF_STMT,
  NK_WHILE_STMT,
  NK_FOR_STMT,
  NK_BLOCK_STMT,
  NK_EXPR_STMT,
  NK_EMPTY_STMT,

  // Declarations
  NK_FUNC_DEF,
  NK_VAR_DECL,

  // Expressions
  NK_UNARY_OP,
  NK_BINARY_OP,
  NK_FUNC_CALL,
  NK_ASSIGN_EXPR,
  NK_IDENTIFIER,
  NK_INT_LITERAL,

  NK_UNKNOWN
};

// Base AST node
struct ASTNode {
  NodeKind kind;
  explicit ASTNode(NodeKind k) : kind(k) {}
  virtual ~ASTNode() = default;
  virtual void print(const std::string &prefix = "", bool is_last = true) const;
  static std::shared_ptr<spdlog::logger> get_logger() {
    static auto logger = spdlog::get("parser_logger");
    return logger;
  }
};

// Forward declarations for all node types
struct ProgramNode;
struct ReturnStmtNode;
struct IfStmtNode;
struct EmptyStmtNode;
struct WhileStmtNode;
struct BlockStmtNode;
struct ExprStmtNode;
struct FuncDefNode;
struct VarDeclNode;
struct BinaryOpNode;
struct AssignExprNode;
struct IdentifierNode;
struct IntLiteralNode;

// Node for the entire program (a list of top-level declarations)
struct ProgramNode : ASTNode {
  std::vector<std::unique_ptr<ASTNode>> declarations;
  ProgramNode() : ASTNode(NodeKind::NK_PROGRAM) {}
  void print(const std::string &prefix = "",
             bool is_last = true) const override;
};

// Node for a return statement
struct ReturnStmtNode : ASTNode {
  std::unique_ptr<ASTNode> expression;
  explicit ReturnStmtNode(std::unique_ptr<ASTNode> expr)
      : ASTNode(NodeKind::NK_RETURN_STMT), expression(std::move(expr)) {}
  void print(const std::string &prefix = "",
             bool is_last = true) const override;
};

// Node for an if-else statement
struct IfStmtNode : ASTNode {
  std::unique_ptr<ASTNode> condition;
  std::unique_ptr<ASTNode> then_branch;
  std::unique_ptr<ASTNode> else_branch; // Can be nullptr
  IfStmtNode(std::unique_ptr<ASTNode> cond, std::unique_ptr<ASTNode> then_br,
             std::unique_ptr<ASTNode> else_br = nullptr)
      : ASTNode(NodeKind::NK_IF_STMT), condition(std::move(cond)),
        then_branch(std::move(then_br)), else_branch(std::move(else_br)) {}
  void print(const std::string &prefix = "",
             bool is_last = true) const override;
};

// Node for a while statement
struct WhileStmtNode : ASTNode {
  std::unique_ptr<ASTNode> condition;
  std::unique_ptr<ASTNode> body;
  WhileStmtNode(std::unique_ptr<ASTNode> cond, std::unique_ptr<ASTNode> body)
      : ASTNode(NodeKind::NK_WHILE_STMT), condition(std::move(cond)),
        body(std::move(body)) {}
  void print(const std::string &prefix = "",
             bool is_last = true) const override;
};

// Node for an empty statement (just a semicolon ;)
struct EmptyStmtNode : ASTNode {
  EmptyStmtNode() : ASTNode(NodeKind::NK_EMPTY_STMT) {}
  void print(const std::string &prefix = "",
             bool is_last = true) const override;
};

// Node for a block of statements { ... }
struct BlockStmtNode : ASTNode {
  std::vector<std::unique_ptr<ASTNode>> statements;
  BlockStmtNode() : ASTNode(NodeKind::NK_BLOCK_STMT) {}
  void print(const std::string &prefix = "",
             bool is_last = true) const override;
};

// Node for an expression used as a statement
struct ExprStmtNode : ASTNode {
  std::unique_ptr<ASTNode> expression;
  explicit ExprStmtNode(std::unique_ptr<ASTNode> expr)
      : ASTNode(NodeKind::NK_EXPR_STMT), expression(std::move(expr)) {}
  void print(const std::string &prefix = "",
             bool is_last = true) const override;
};

// Node for a function definition
struct FuncDefNode : ASTNode {
  TOKEN_TYPE return_type;
  std::string name;
  std::vector<std::unique_ptr<VarDeclNode>> parameters;
  std::unique_ptr<ASTNode> body; // Should be a BlockStmtNode
  FuncDefNode(TOKEN_TYPE ret_type, std::string n,
              std::vector<std::unique_ptr<VarDeclNode>> params,
              std::unique_ptr<ASTNode> b)
      : ASTNode(NodeKind::NK_FUNC_DEF), return_type(ret_type),
        name(std::move(n)), parameters(std::move(params)), body(std::move(b)) {}
  void print(const std::string &prefix = "",
             bool is_last = true) const override;
};

// Node for a variable declaration
struct VarDeclNode : ASTNode {
  std::string name;
  std::unique_ptr<ASTNode> initializer; // Can be nullptr
  VarDeclNode(std::string n, std::unique_ptr<ASTNode> init = nullptr)
      : ASTNode(NodeKind::NK_VAR_DECL), name(std::move(n)),
        initializer(std::move(init)) {}
  void print(const std::string &prefix = "",
             bool is_last = true) const override;
};

// Node for a binary operation (e.g., a + b, a == b)
struct BinaryOpNode : ASTNode {
  Token op;
  std::unique_ptr<ASTNode> left;
  std::unique_ptr<ASTNode> right;
  BinaryOpNode(Token op, std::unique_ptr<ASTNode> left,
               std::unique_ptr<ASTNode> right)
      : ASTNode(NodeKind::NK_BINARY_OP), op(op), left(std::move(left)),
        right(std::move(right)) {}
  BinaryOpNode() = delete;
  void print(const std::string &prefix = "",
             bool is_last = true) const override;
};

// Node for a function call (e.g., f(a, b))
struct CallExprNode : ASTNode {
  std::string callee;
  std::vector<std::unique_ptr<ASTNode>> arguments;
  CallExprNode(std::string name, std::vector<std::unique_ptr<ASTNode>> args)
      : ASTNode(NodeKind::NK_FUNC_CALL), callee(std::move(name)),
        arguments(std::move(args)) {}
  void print(const std::string &prefix = "",
             bool is_last = true) const override;
};

// Node for an assignment expression (e.g., a = b)
struct AssignExprNode : ASTNode {
  std::unique_ptr<ASTNode> target;
  std::unique_ptr<ASTNode> value;
  AssignExprNode(std::unique_ptr<ASTNode> tgt, std::unique_ptr<ASTNode> val)
      : ASTNode(NodeKind::NK_ASSIGN_EXPR), target(std::move(tgt)),
        value(std::move(val)) {}
  void print(const std::string &prefix = "",
             bool is_last = true) const override;
};

// Node for an identifier
struct IdentifierNode : ASTNode {
  std::string name;
  explicit IdentifierNode(std::string n)
      : ASTNode(NodeKind::NK_IDENTIFIER), name(std::move(n)) {}
  void print(const std::string &prefix = "",
             bool is_last = true) const override;
};

// Node for an integer literal
struct IntLiteralNode : ASTNode {
  long long value;
  explicit IntLiteralNode(long long val)
      : ASTNode(NodeKind::NK_INT_LITERAL), value(val) {}
  void print(const std::string &prefix = "",
             bool is_last = true) const override;
};

} // namespace minicc
