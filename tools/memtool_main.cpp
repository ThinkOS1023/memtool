#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "memtool_memory.h"
#include "memtool_ui.h"
#include "memtool_util.h"

int main(int argc, char** argv) {
  pid_t pid = 0;
  std::string command;
  if (argc == 1) {
    command = "ui";
    pid = 0;
  } else if (argc == 2) {
    command = "ui";
    if (!ParsePid(argv[1], &pid)) {
      std::fprintf(stderr, "pid 无效。\n");
      return 1;
    }
  } else {
    command = argv[1];
    if (command != "read" && command != "write" && command != "ui") {
      PrintUsage(argv[0]);
      return 1;
    }
    if ((command == "read" && argc != 5) || (command == "write" && argc != 5) ||
        (command == "ui" && argc != 3)) {
      PrintUsage(argv[0]);
      return 1;
    }
    if (!ParsePid(argv[2], &pid)) {
      std::fprintf(stderr, "pid 无效。\n");
      return 1;
    }
  }

  int exit_code = 0;
  if (command == "read") {
    std::uintptr_t addr = 0;
    size_t len = 0;
    if (!ParseAddress(argv[3], &addr) || !ParseLength(argv[4], &len)) {
      std::fprintf(stderr, "地址或长度无效。\n");
      return 1;
    }
    std::vector<unsigned char> buf;
    std::string err;
    if (!ReadMemory(pid, addr, len, &buf, &err)) {
      std::fprintf(stderr, "%s\n", err.c_str());
      exit_code = 1;
    } else {
      PrintHex(buf);
    }
  } else if (command == "write") {
    std::uintptr_t addr = 0;
    if (!ParseAddress(argv[3], &addr)) {
      std::fprintf(stderr, "地址无效。\n");
      return 1;
    }
    std::vector<unsigned char> bytes;
    if (!ParseHexBytes(argv[4], &bytes)) {
      std::fprintf(stderr,
                   "十六进制字节无效（需要偶数长度的十六进制字符串）。\n");
      return 1;
    }
    std::string err;
    if (!WriteMemory(pid, addr, bytes, &err)) {
      std::fprintf(stderr, "%s\n", err.c_str());
      exit_code = 1;
    }
  } else {
    RunUi(pid);
  }

  return exit_code;
}
