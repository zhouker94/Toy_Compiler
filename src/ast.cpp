#include "spdlog/spdlog.h"

#include "parser.hpp"
#include "utils.hpp"

namespace minicc {

void ProgramNode::print(const std::string &prefix, bool is_last) const {
  get_logger()->info("{}Program", prefix);
  for (size_t i = 0; i < declarations.size(); ++i) {
    declarations[i]->print(prefix + "  ", i == declarations.size() - 1);
  }
}

void CallExprNode::print(const std::string &prefix, bool is_last) const {
  get_logger()->info("{}{}Function Call: {}", prefix,
                     (is_last ? "└── " : "├── "), callee);
  std::string new_prefix = prefix + (is_last ? "    " : "│   ");
  for (size_t i = 0; i < arguments.size(); ++i) {
    arguments[i]->print(new_prefix, i == arguments.size() - 1);
  }
}

void FuncDefNode::print(const std::string &prefix, bool is_last) const {
  get_logger()->info("{}{}Function Definition: {} {}", prefix,
                     (is_last ? "└── " : "├── "),
                     token_type_to_string(return_type), name);
  std::string new_prefix = prefix + (is_last ? "    " : "│   ");
  for (size_t i = 0; i < parameters.size(); ++i) {
    parameters[i]->print(new_prefix, false);
  }
  body->print(new_prefix, true);
}

void EmptyStmtNode::print(const std::string &prefix, bool is_last) const {
  get_logger()->info("{}{}Empty Statement (;)", prefix,
                     (is_last ? "└── " : "├── "));
}

void BlockStmtNode::print(const std::string &prefix, bool is_last) const {
  get_logger()->info("{}{}Block Statement", prefix,
                     (is_last ? "└── " : "├── "));
  std::string new_prefix = prefix + (is_last ? "    " : "│   ");
  for (size_t i = 0; i < statements.size(); ++i) {
    statements[i]->print(new_prefix, i == statements.size() - 1);
  }
}

void ReturnStmtNode::print(const std::string &prefix, bool is_last) const {
  get_logger()->info("{}{}Return Statement", prefix,
                     (is_last ? "└── " : "├── "));
  expression->print(prefix + (is_last ? "    " : "│   "), true);
}

void IntLiteralNode::print(const std::string &prefix, bool is_last) const {
  get_logger()->info("{}{}Integer Literal: {}", prefix,
                     (is_last ? "└── " : "├── "), value);
}

void IfStmtNode::print(const std::string &prefix, bool is_last) const {
  get_logger()->info("{}{}If Statement", prefix, (is_last ? "└── " : "├── "));
  std::string new_prefix = prefix + (is_last ? "    " : "│   ");
  condition->print(new_prefix, false);
  then_branch->print(new_prefix, else_branch == nullptr);
  if (else_branch) {
    else_branch->print(new_prefix, true);
  }
}

void WhileStmtNode::print(const std::string &prefix, bool is_last) const {
  get_logger()->info("{}{}While Statement", prefix,
                     (is_last ? "└── " : "├── "));
  std::string new_prefix = prefix + (is_last ? "    " : "│   ");
  condition->print(new_prefix, false);
  body->print(new_prefix, true);
}

void ExprStmtNode::print(const std::string &prefix, bool is_last) const {
  get_logger()->info("{}{}Expression Statement", prefix,
                     (is_last ? "└── " : "├── "));
  expression->print(prefix + (is_last ? "    " : "│   "), true);
}

void VarDeclNode::print(const std::string &prefix, bool is_last) const {
  get_logger()->info("{}{}Variable Declaration: {}", prefix,
                     (is_last ? "└── " : "├── "), name);
  if (initializer) {
    initializer->print(prefix + (is_last ? "    " : "│   "), true);
  }
}

void BinaryOpNode::print(const std::string &prefix, bool is_last) const {
  get_logger()->info("{}{}Binary Operation: {}", prefix,
                     (is_last ? "└── " : "├── "),
                     token_type_to_string(op.type));
  std::string new_prefix = prefix + (is_last ? "    " : "│   ");
  left->print(new_prefix, false);
  right->print(new_prefix, true);
}

void AssignExprNode::print(const std::string &prefix, bool is_last) const {
  get_logger()->info("{}{}Assignment Expression", prefix,
                     (is_last ? "└── " : "├── "));
  std::string new_prefix = prefix + (is_last ? "    " : "│   ");
  target->print(new_prefix, false);
  value->print(new_prefix, true);
}

void IdentifierNode::print(const std::string &prefix, bool is_last) const {
  get_logger()->info("{}{}Identifier: {}", prefix, (is_last ? "└── " : "├── "),
                     name);
}

void ASTNode::print(const std::string &prefix, bool is_last) const {
  get_logger()->debug("{}{}Node (type: {})", prefix,
                      (is_last ? "└── " : "├── "), static_cast<int>(kind));
}

} // namespace minicc
