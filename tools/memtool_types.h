#pragma once

#include <cstdint>
#include <string>

struct MapRegion {
  std::uintptr_t start = 0;
  std::uintptr_t end = 0;
  std::string perms;
  std::string name;
};
