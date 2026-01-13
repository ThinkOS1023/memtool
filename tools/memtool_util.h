#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <sys/types.h>

bool ParsePid(const char* s, pid_t* out);
bool ParseAddress(const char* s, std::uintptr_t* out);
bool ParseLength(const char* s, size_t* out);
bool ParseHexBytes(const char* s, std::vector<unsigned char>* out);
bool ParseU64(const std::string& s, std::uint64_t* out);

std::vector<std::string> SplitTokens(const std::string& line);
void PrintHex(const std::vector<unsigned char>& data);
