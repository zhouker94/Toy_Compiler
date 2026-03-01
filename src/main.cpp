#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "ir_generator.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "utils.hpp"

int main(int argc, const char *argv[]) {
  bool verbose = false;
  std::string input_filename = "examples/simple.c";
  std::string output_filename;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--verbose") {
      verbose = true;
    } else {
      input_filename = arg;
    }
  }

  auto parser_logger = minicc::create_console_logger("parser_logger", verbose);
  auto lexer_logger = minicc::create_console_logger("lexer_logger", verbose);
  auto main_logger = minicc::create_console_logger("main_logger", verbose);

  std::ifstream infile = minicc::open_file(input_filename);
  if (!infile) {
    main_logger->error("Read input file failed.");
    return 1;
  }

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
    std::stringstream ir_stream;
    ir_gen.generate(ast.get(), ir_stream);
    std::string ir_code = ir_stream.str();

    // --- Backend Steps ---
    std::string out_dir = "bin";
    std::filesystem::create_directory(out_dir);

    std::string base_name = minicc::get_base_filename(input_filename);
    std::string ll_filename = out_dir + "/" + base_name + ".ll";
    std::string obj_filename = out_dir + "/" + base_name + ".o";
    output_filename = out_dir + "/" + base_name; // Final executable name

    // 1. Save IR to .ll file
    main_logger->info("1. Writing LLVM IR to '{}'...", ll_filename);
    std::ofstream ll_file(ll_filename);
    ll_file << ir_code;
    ll_file.close();

    // 2. Compile .ll to .o using llc
    const char *llc_env = std::getenv("LLC_COMMAND");
    std::string llc_executable = (llc_env) ? llc_env : "llc";

    std::string llc_command = llc_executable + " -filetype=obj " + ll_filename +
                              " -o " + obj_filename;

    main_logger->info("2. Compiling IR to object file with: {}", llc_command);
    if (system(llc_command.c_str()) != 0) {
      throw std::runtime_error("llc compilation failed.");
    }

    // 3. Link .o to executable using clang
    const char *clang_env = std::getenv("CLANG_COMMAND");
    std::string clang_executable = (clang_env) ? clang_env : "clang";

    std::string link_command =
        clang_executable + " " + obj_filename + " -o " + output_filename;

    main_logger->info("3. Linking object file to executable with: {}",
                      link_command);
    if (system(link_command.c_str()) != 0) {
      throw std::runtime_error("Linking failed.");
    }

    main_logger->info("✅ Successfully compiled '{}' to '{}'", input_filename,
                      output_filename);
  } catch (const std::exception &e) {
    main_logger->error(e.what());
    infile.close();
    return 1;
  }

  infile.close();

  return 0;
}
