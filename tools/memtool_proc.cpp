#include "memtool_proc.h"

#include <cinttypes>
#include <cstdio>
#include <cstring>

#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include "memtool_util.h"

std::vector<MapRegion> ReadMaps(pid_t pid) {
  std::vector<MapRegion> regions;
  char path[64];
  std::snprintf(path, sizeof(path), "/proc/%d/maps", pid);
  FILE* fp = std::fopen(path, "r");
  if (!fp) {
    return regions;
  }
  char line[512];
  while (std::fgets(line, sizeof(line), fp)) {
    std::string l(line);
    std::uintptr_t start = 0;
    std::uintptr_t end = 0;
    char perms[8] = {};
    char name[256] = {};
    int read = std::sscanf(
        l.c_str(), "%" SCNxPTR "-%" SCNxPTR " %7s %*s %*s %*s %255[^\n]", &start,
        &end, perms, name);
    if (read >= 3) {
      MapRegion region;
      region.start = start;
      region.end = end;
      region.perms = perms;
      if (read == 4) {
        region.name = name;
      }
      regions.push_back(region);
    }
  }
  std::fclose(fp);
  return regions;
}

std::vector<pid_t> FindPidsByPackage(const std::string& package) {
  std::vector<pid_t> pids;
  DIR* dir = opendir("/proc");
  if (!dir) {
    return pids;
  }
  dirent* ent = nullptr;
  while ((ent = readdir(dir)) != nullptr) {
    if (ent->d_type != DT_DIR && ent->d_type != DT_UNKNOWN) {
      continue;
    }
    pid_t pid = 0;
    if (!ParsePid(ent->d_name, &pid)) {
      continue;
    }
    char path[64];
    std::snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
    FILE* fp = std::fopen(path, "r");
    if (!fp) {
      continue;
    }
    char buf[256] = {};
    size_t n = std::fread(buf, 1, sizeof(buf) - 1, fp);
    std::fclose(fp);
    if (n == 0) {
      continue;
    }
    std::string cmdline(buf);
    if (cmdline == package) {
      pids.push_back(pid);
    }
  }
  closedir(dir);
  return pids;
}

bool IsElfFile(const std::string& path) {
  int fd = open(path.c_str(), O_RDONLY | O_CLOEXEC);
  if (fd == -1) {
    return false;
  }
  unsigned char magic[4] = {};
  ssize_t got = read(fd, magic, sizeof(magic));
  close(fd);
  return got == 4 && magic[0] == 0x7f && magic[1] == 'E' && magic[2] == 'L' &&
         magic[3] == 'F';
}
