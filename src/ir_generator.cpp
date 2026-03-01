#include "ir_generator.hpp"

#include "ast.hpp"
#include "utils.hpp"

namespace minicc {

IRGenerator::IRGenerator()
    : out_(nullptr), temp_counter_(0), label_counter_(0),
      block_terminated_(false), is_in_function_scope_(false) {}

void IRGenerator::generate(const ASTNode *root, std::ostream &out) {
  out_ = &out;
  temp_counter_ = 0;
  label_counter_ = 0;
  global_symbol_table_.clear();
  local_symbol_table_.clear();
  function_return_types_.clear();

  // 1. Emit LLVM IR Header
  emit_raw("; ModuleID = 'minicc'");
  emit_raw("source_filename = \"minicc\"");
  emit_raw("");

  is_in_function_scope_ = false;
  // 2. Start traversing the AST
  if (root) {
    visit(root);
  }
}

std::string IRGenerator::next_temp() {
  return "%t" + std::to_string(temp_counter_++);
}

std::string IRGenerator::next_label() {
  return "L" + std::to_string(label_counter_++);
}

void IRGenerator::emit(const std::string &instr) {
  *out_ << "  " << instr << "\n";
}

void IRGenerator::emit_raw(const std::string &content) {
  *out_ << content << "\n";
}

std::string IRGenerator::visit(const ASTNode *node) {
  switch (node->kind) {
  // Statements (and Program/FuncDef)
  case NodeKind::NK_PROGRAM:
    visit(static_cast<const ProgramNode *>(node));
    break;
  case NodeKind::NK_FUNC_DEF:
    visit(static_cast<const FuncDefNode *>(node));
    break;
  case NodeKind::NK_BLOCK_STMT:
    visit(static_cast<const BlockStmtNode *>(node));
    break;
  case NodeKind::NK_RETURN_STMT:
    visit(static_cast<const ReturnStmtNode *>(node));
    break;
  case NodeKind::NK_VAR_DECL:
    visit(static_cast<const VarDeclNode *>(node));
    break;
  case NodeKind::NK_IF_STMT:
    visit(static_cast<const IfStmtNode *>(node));
    break;
  case NodeKind::NK_WHILE_STMT:
    visit(static_cast<const WhileStmtNode *>(node));
    break;
  case NodeKind::NK_EXPR_STMT:
    visit(static_cast<const ExprStmtNode *>(node));
    break;
  case NodeKind::NK_EMPTY_STMT:
    visit(static_cast<const EmptyStmtNode *>(node));
    break;

  // Expressions
  case NodeKind::NK_BINARY_OP:
    return visit(static_cast<const BinaryOpNode *>(node));
  case NodeKind::NK_INT_LITERAL:
    return visit(static_cast<const IntLiteralNode *>(node));
  case NodeKind::NK_IDENTIFIER:
    return visit(static_cast<const IdentifierNode *>(node));
  case NodeKind::NK_ASSIGN_EXPR:
    return visit(static_cast<const AssignExprNode *>(node));
  case NodeKind::NK_FUNC_CALL:
    return visit(static_cast<const CallExprNode *>(node));

  default:
    emit_raw("; WARNING: Unhandled AST node type: " +
             std::to_string(static_cast<int>(node->kind)));
  }
  return ""; // Return empty for statements
}

// Visitor implementations
void IRGenerator::visit(const ProgramNode *node) {
  for (const auto &decl : node->declarations) {
    visit(decl.get());
  }
}

void IRGenerator::visit(const FuncDefNode *node) {
  is_in_function_scope_ = true;
  block_terminated_ = false;
  local_symbol_table_.clear();
  temp_counter_ = 0;
  function_return_types_[node->name] = node->return_type;

  std::string return_type_str = "i32";
  if (node->return_type == T_VOID) {
    return_type_str = "void";
  }

  // 1. Process parameters
  std::vector<std::string> param_types;
  for (size_t i = 0; i < node->parameters.size(); ++i) {
    param_types.push_back("i32"); // Assuming all params are i32
  }

  // 2. Emit function signature
  emit_raw("define " + return_type_str + " @" + node->name + "(" +
           join(param_types, ", ") + ") {");
  emit_raw("entry:");

  // 3. Allocate space for parameters and store their initial values
  for (size_t i = 0; i < node->parameters.size(); ++i) {
    const auto &param = node->parameters[i];
    std::string var_name = param->name;
    std::string var_addr = "%" + var_name + ".addr";
    local_symbol_table_[var_name] = var_addr;

    emit(var_addr + " = alloca i32");
    emit("store i32 %" + std::to_string(i) + ", i32* " + var_addr);
  }

  // 4. Visit the function body
  visit(node->body.get());

  // 5. Ensure a return instruction for functions that might lack one
  if (!block_terminated_) {
    if (node->return_type == T_VOID) {
      emit("ret void");
    } else {
      // For non-void functions, LLVM requires a return.
      // A well-formed program should already have one, but we add a default
      // one to prevent malformed IR. This can happen if control flows off
      // the end of a non-void function.
      emit("ret i32 0");
    }
  }

  emit_raw("}");
  emit_raw(""); // Newline for readability
  is_in_function_scope_ = false;
}

void IRGenerator::visit(const BlockStmtNode *node) {
  for (const auto &stmt : node->statements) {
    visit(stmt.get());
    if (block_terminated_) {
      // Stop generating code for statements in this block after a terminator.
      break;
    }
  }
}

void IRGenerator::visit(const ReturnStmtNode *node) {
  if (block_terminated_)
    return;
  if (node->expression) {
    std::string result = visit(node->expression.get());
    emit("ret i32 " + result);
  } else {
    emit("ret void");
  }
  block_terminated_ = true;
}

void IRGenerator::visit(const VarDeclNode *node) {
  if (is_in_function_scope_) {
    // Local variable
    std::string var_addr = "%" + node->name + ".addr";
    emit(var_addr + " = alloca i32"); // Assuming i32 for now
    local_symbol_table_[node->name] = var_addr;

    if (node->initializer) {
      std::string init_val = visit(node->initializer.get());
      emit("store i32 " + init_val + ", i32* " + var_addr);
    }
  } else {
    // Global variable
    std::string var_addr = "@" + node->name;
    global_symbol_table_[node->name] = var_addr;
    // Note: Initializers for globals must be constants. This toy compiler
    // doesn't have a constant expression evaluator, so we assume 0 or error.
    // A real compiler would have a semantic check for this.
    if (node->initializer) {
      auto *int_literal =
          dynamic_cast<const IntLiteralNode *>(node->initializer.get());
      if (int_literal) {
        emit_raw(var_addr + " = global i32 " +
                 std::to_string(int_literal->value));
      } else {
        emit_raw("; ERROR: Global variable initializer for '" + node->name +
                 "' is not a constant expression.");
        emit_raw(var_addr + " = global i32 0");
      }
    } else {
      emit_raw(var_addr + " = common global i32 0, align 4");
    }
  }
}

void IRGenerator::visit(const IfStmtNode *node) {
  std::string then_label = next_label();
  std::string else_label = next_label();
  std::string merge_label = next_label();
  bool then_terminated = false, else_terminated = false;

  // 1. Evaluate the condition
  std::string cond_val = visit(node->condition.get());

  // 2. Branch based on the condition
  if (node->else_branch) {
    emit("br i1 " + cond_val + ", label %" + then_label + ", label %" +
         else_label);
  } else {
    emit("br i1 " + cond_val + ", label %" + then_label + ", label %" +
         merge_label);
  }

  // 3. Emit the 'then' block
  emit_raw(then_label + ":");
  block_terminated_ = false;
  visit(node->then_branch.get());
  if (!block_terminated_) {
    emit("br label %" + merge_label);
  }
  then_terminated = block_terminated_;

  // 4. Emit the 'else' block if it exists
  if (node->else_branch) {
    emit_raw(else_label + ":");
    block_terminated_ = false;
    visit(node->else_branch.get());
    if (!block_terminated_) {
      emit("br label %" + merge_label);
    }
    else_terminated = block_terminated_;
  } else {
    else_terminated = false; // Implicit else flows to merge block.
  }

  // 5. Emit the merge block label
  emit_raw(merge_label + ":");
  block_terminated_ = then_terminated && else_terminated;
}

void IRGenerator::visit(const WhileStmtNode *node) {
  std::string cond_label = next_label();
  std::string body_label = next_label();
  std::string exit_label = next_label();

  // 1. Jump to the condition check
  emit("br label %" + cond_label);

  // 2. Emit the condition-checking block
  emit_raw(cond_label + ":");
  block_terminated_ = false;
  std::string cond_val = visit(node->condition.get());
  emit("br i1 " + cond_val + ", label %" + body_label + ", label %" +
       exit_label);

  // 3. Emit the loop body block
  emit_raw(body_label + ":");
  block_terminated_ = false;
  visit(node->body.get());
  if (!block_terminated_) {
    emit("br label %" + cond_label); // Loop back to the condition
  }

  // 4. Emit the exit block label
  emit_raw(exit_label + ":");
  block_terminated_ = false;
}

void IRGenerator::visit(const ExprStmtNode *node) {
  // Visit the expression to generate its code, but discard the result.
  visit(node->expression.get());
}

void IRGenerator::visit(const EmptyStmtNode *node) {
  (void)node; // unused
  // An empty statement does nothing.
}

std::string IRGenerator::visit(const BinaryOpNode *node) {
  std::string lhs = visit(node->left.get());
  std::string rhs = visit(node->right.get());
  std::string result = next_temp();

  switch (node->op.type) {
  // Arithmetic
  case T_PLUS:
    emit(result + " = add i32 " + lhs + ", " + rhs);
    break;
  case T_MINUS:
    emit(result + " = sub i32 " + lhs + ", " + rhs);
    break;
  case T_STAR:
    emit(result + " = mul i32 " + lhs + ", " + rhs);
    break;
  case T_SLASH:
    emit(result + " = sdiv i32 " + lhs + ", " + rhs);
    break;

  // Comparison
  case T_EQ:
    emit(result + " = icmp eq i32 " + lhs + ", " + rhs);
    break;
  case T_NEQ:
    emit(result + " = icmp ne i32 " + lhs + ", " + rhs);
    break;
  case T_LT:
    emit(result + " = icmp slt i32 " + lhs + ", " + rhs);
    break;
  case T_LTE:
    emit(result + " = icmp sle i32 " + lhs + ", " + rhs);
    break;
  case T_GT:
    emit(result + " = icmp sgt i32 " + lhs + ", " + rhs);
    break;
  case T_GTE:
    emit(result + " = icmp sge i32 " + lhs + ", " + rhs);
    break;

  default:
    emit_raw("; WARNING: Unhandled binary operator: " + node->op.text);
    // Return 0 or some other default to avoid malformed IR
    return "0";
  }

  return result;
}

std::string IRGenerator::visit(const IntLiteralNode *node) {
  return std::to_string(node->value);
}

std::string IRGenerator::visit(const IdentifierNode *node) {
  // 1. Find the variable's address in the symbol table, local scope first.
  std::string var_addr;
  if (is_in_function_scope_) {
    auto it = local_symbol_table_.find(node->name);
    if (it != local_symbol_table_.end()) {
      var_addr = it->second;
    }
  }
  // 2. If not found in local, check global scope.
  if (var_addr.empty()) {
    auto it = global_symbol_table_.find(node->name);
    if (it != global_symbol_table_.end()) {
      var_addr = it->second;
    } else {
      emit_raw("; ERROR: Undefined variable '" + node->name + "'");
      return "0"; // Should be handled by a semantic analysis pass
    }
  }

  // 2. Load the value from the address into a new temporary
  std::string result = next_temp();
  emit(result + " = load i32, i32* " + var_addr);
  return result;
}

std::string IRGenerator::visit(const AssignExprNode *node) {
  // The LHS of an assignment must be something we can assign to,
  // which for this toy compiler is just an identifier.
  auto *target_node = static_cast<const IdentifierNode *>(node->target.get());
  std::string var_addr;
  // 1. Find the variable's address in the symbol table, local scope first.
  if (is_in_function_scope_) {
    auto it = local_symbol_table_.find(target_node->name);
    if (it != local_symbol_table_.end()) {
      var_addr = it->second;
    }
  }
  // 2. If not found in local, check global scope.
  if (var_addr.empty()) {
    auto it = global_symbol_table_.find(target_node->name);
    if (it != global_symbol_table_.end()) {
      var_addr = it->second;
    } else {
      emit_raw("; ERROR: Assignment to undeclared variable '" +
               target_node->name + "'");
      return "0";
    }
  }
  // 1. Visit the RHS to get the value to store
  std::string val = visit(node->value.get());

  // 2. Store the value
  emit("store i32 " + val + ", i32* " + var_addr);

  // 3. The result of an assignment expression is the value assigned
  return val;
}

std::string IRGenerator::visit(const CallExprNode *node) {
  // 1. Visit arguments to get their resulting registers/values
  std::vector<std::string> arg_irs;
  for (const auto &arg : node->arguments) {
    std::string arg_val = visit(arg.get());
    arg_irs.push_back("i32 " + arg_val);
  }

  // 2. Construct the call instruction
  TOKEN_TYPE return_type = T_INT; // Default assumption for undeclared functions
  auto it = function_return_types_.find(node->callee);
  if (it != function_return_types_.end()) {
    return_type = it->second;
  } else {
    // This could be a call to an external, undeclared function.
    // A proper compiler would handle this via declarations.
    // We'll warn and assume i32, which is a common C legacy behavior.
    emit_raw("; WARNING: Assuming i32 return type for undeclared function @" +
             node->callee);
  }

  if (return_type == T_VOID) {
    emit("call void @" + node->callee + "(" + join(arg_irs, ", ") + ")");
    return ""; // Expressions expecting a value from a void call is a semantic
               // error.
  } else {
    std::string result = next_temp();
    emit(result + " = call i32 @" + node->callee + "(" + join(arg_irs, ", ") +
         ")");
    return result;
  }
}

} // namespace minicc
