#include "memtool_util.h"

#include <cstdio>
#include <cstdlib>
#include <sstream>

bool ParsePid(const char* s, pid_t* out) {
  char* end = nullptr;
  long val = std::strtol(s, &end, 10);
  if (!end || *end != '\0' || val <= 0) {
    return false;
  }
  *out = static_cast<pid_t>(val);
  return true;
}

bool ParseAddress(const char* s, std::uintptr_t* out) {
  char* end = nullptr;
  std::uintptr_t val = std::strtoull(s, &end, 0);
  if (!end || *end != '\0') {
    return false;
  }
  *out = val;
  return true;
}

bool ParseLength(const char* s, size_t* out) {
  char* end = nullptr;
  unsigned long val = std::strtoul(s, &end, 10);
  if (!end || *end != '\0' || val == 0) {
    return false;
  }
  *out = static_cast<size_t>(val);
  return true;
}

bool ParseHexBytes(const char* s, std::vector<unsigned char>* out) {
  std::string input(s);
  if (input.rfind("0x", 0) == 0 || input.rfind("0X", 0) == 0) {
    input = input.substr(2);
  }
  if (input.empty() || (input.size() % 2) != 0) {
    return false;
  }
  out->clear();
  out->reserve(input.size() / 2);
  for (size_t i = 0; i < input.size(); i += 2) {
    char buf[3] = {input[i], input[i + 1], '\0'};
    char* end = nullptr;
    long byte = std::strtol(buf, &end, 16);
    if (!end || *end != '\0' || byte < 0 || byte > 255) {
      return false;
    }
    out->push_back(static_cast<unsigned char>(byte));
  }
  return true;
}

bool ParseU64(const std::string& s, std::uint64_t* out) {
  char* end = nullptr;
  unsigned long long val = std::strtoull(s.c_str(), &end, 0);
  if (!end || *end != '\0') {
    return false;
  }
  *out = static_cast<std::uint64_t>(val);
  return true;
}

std::vector<std::string> SplitTokens(const std::string& line) {
  std::istringstream iss(line);
  std::vector<std::string> out;
  std::string tok;
  while (iss >> tok) {
    out.push_back(tok);
  }
  return out;
}

void PrintHex(const std::vector<unsigned char>& data) {
  for (size_t i = 0; i < data.size(); ++i) {
    std::printf("%02x", data[i]);
  }
  std::printf("\n");
}
