#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "spdlog/spdlog.h"

namespace minicc {

// open a file and return std::ifstream; the caller can check !stream for
// failure
std::ifstream open_file(const std::string &path);

// split string by delimiter, skipping empty tokens
std::vector<std::string> split(const std::string &s, char delim);

// trim whitespace from both ends (also strips trailing ';')
void trim(std::string &str);

// join a vector of strings with a separator
std::string join(const std::vector<std::string> &vec,
                 const std::string &separator);

// Creates and configures a console logger with the given name and verbosity.
std::shared_ptr<spdlog::logger> create_console_logger(const std::string &name,
                                                      bool verbose);

// get base name from path, e.g., "examples/simple.c" -> "simple"
std::string get_base_filename(const std::string &path);

} // namespace minicc
