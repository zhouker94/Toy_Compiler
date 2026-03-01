#include "utils.hpp"

#include "spdlog/sinks/stdout_color_sinks.h"
#include <iostream>
#include <sstream>

namespace minicc {

std::ifstream open_file(const std::string &path) {
  std::ifstream infile(path);
  if (!infile) {
    std::cerr << "Cannot open " << path << std::endl;
  }
  return infile;
}

std::vector<std::string> split(const std::string &s, char delim) {
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> tokens;
  while (std::getline(ss, item, delim)) {
    if (!item.empty())
      tokens.push_back(item);
  }
  return tokens;
}

void trim(std::string &str) {
  size_t first = str.find_first_not_of(" \n\r\t");
  size_t last = str.find_last_not_of(" \n\r\t;");
  if (first == std::string::npos || last == std::string::npos) {
    str.clear();
    return;
  }
  str = str.substr(first, (last - first + 1));
}

std::shared_ptr<spdlog::logger> create_console_logger(const std::string &name,
                                                      bool verbose) {
  auto logger = spdlog::get(name);
  if (!logger) {
    logger = spdlog::stdout_color_mt(name);
  }
  if (verbose) {
    logger->set_level(spdlog::level::info);
  } else {
    logger->set_level(spdlog::level::warn);
  }
  return logger;
}

std::string join(const std::vector<std::string> &vec,
                 const std::string &separator) {
  std::stringstream ss;
  for (size_t i = 0; i < vec.size(); ++i) {
    if (i > 0) {
      ss << separator;
    }
    ss << vec[i];
  }
  return ss.str();
}

} // namespace minicc
