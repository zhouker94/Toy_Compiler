#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

// Forward declarations for all AST nodes
namespace minicc {
struct ASTNode;
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
struct CallExprNode;
struct AssignExprNode;
struct IdentifierNode;
struct IntLiteralNode;
} // namespace minicc

namespace minicc {

class IRGenerator {
public:
  IRGenerator();

  // Entry point: Generate LLVM IR from the AST root to the output stream
  void generate(const ASTNode *root, std::ostream &out = std::cout);

private:
  std::ostream *out_;
  int temp_counter_;
  int label_counter_;
  std::map<std::string, std::string>
      symbol_table_; // Maps variable name to register

  // Helper methods for IR generation
  std::string
  next_temp(); // Generate unique temporary register name (e.g., %t1)
  std::string next_label();            // Generate unique label name (e.g., L1)
  void emit(const std::string &instr); // Emit indented instruction
  void
  emit_raw(const std::string &content); // Emit raw content (labels, headers)

  // Main visitor dispatch
  std::string visit(const ASTNode *node);

  // Visitors for different node types
  void visit(const ProgramNode *node);
  void visit(const FuncDefNode *node);
  void visit(const BlockStmtNode *node);
  void visit(const ReturnStmtNode *node);
  void visit(const VarDeclNode *node);
  void visit(const IfStmtNode *node);
  void visit(const WhileStmtNode *node);
  void visit(const ExprStmtNode *node);
  void visit(const EmptyStmtNode *node);

  std::string visit(const BinaryOpNode *node);
  std::string visit(const IntLiteralNode *node);
  std::string visit(const IdentifierNode *node);
  std::string visit(const AssignExprNode *node);
  std::string visit(const CallExprNode *node);
};

} // namespace minicc
