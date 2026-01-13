#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <sys/types.h>

bool ReadMemory(pid_t pid, std::uintptr_t addr, size_t len,
                std::vector<unsigned char>* out, std::string* err);
bool WriteMemory(pid_t pid, std::uintptr_t addr,
                 const std::vector<unsigned char>& bytes, std::string* err);
