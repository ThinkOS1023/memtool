#pragma once

#include <string>
#include <vector>

#include <sys/types.h>

#include "memtool_types.h"

std::vector<MapRegion> ReadMaps(pid_t pid);
std::vector<pid_t> FindPidsByPackage(const std::string& package);
bool IsElfFile(const std::string& path);
