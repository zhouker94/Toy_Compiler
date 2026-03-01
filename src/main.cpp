#include <fstream>
#include <iostream>
#include <string>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "ir_generator.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "utils.hpp"

int main(int argc, const char *argv[]) {
  bool verbose = false;
  std::string filename = "examples/simple.cpp";

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--verbose") {
      verbose = true;
    } else {
      filename = arg;
    }
  }

  std::ifstream infile = minicc::open_file(filename);
  if (!infile)
    return 1;

  auto parser_logger = minicc::create_console_logger("parser_logger", verbose);
  auto lexer_logger = minicc::create_console_logger("lexer_logger", verbose);

  std::string raw_text((std::istreambuf_iterator<char>(infile)),
                       std::istreambuf_iterator<char>());
  try {
    std::vector<minicc::Token> all_tokens = minicc::Lexer::tokenize(raw_text);

    // run parser on the full token stream
    minicc::Parser parser;
    auto ast = parser.parse(all_tokens);

    if (verbose) {
      ast->print();
    }

    // Generate LLVM IR
    minicc::IRGenerator ir_gen;
    ir_gen.generate(ast.get(), std::cout);

  } catch (const std::exception &e) {
    spdlog::error(e.what());
    infile.close();
    return 1;
  }

  infile.close();

  return 0;
}
