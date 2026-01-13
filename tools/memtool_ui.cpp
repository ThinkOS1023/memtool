#include "memtool_ui.h"

#include <algorithm>
#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <iostream>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "memtool_memory.h"
#include "memtool_proc.h"
#include "memtool_types.h"
#include "memtool_util.h"

namespace {

const char* StatusLabel(pid_t pid) {
  return pid > 0 ? "已选进程" : "未选进程";
}

void PrintBanner() {
  std::printf(
      "========================================\n"
      " memtool - 进程内存读写工具 (NDK/Linux)\n"
      " 输入 help 查看命令列表\n"
      "========================================\n");
}

void PrintUiHelp() {
  std::printf(
      "\n命令列表:\n"
      "  help                         显示帮助\n"
      "  pid   <pid|package>          选择进程 (pid 或包名)\n"
      "  pids  <package>              查询包名对应 PID 列表\n"
      "  maps                         显示 /proc/<pid>/maps\n"
      "  elfs                         枚举已加载 ELF\n"
      "  read  <addr_hex> <len_dec>   读取内存\n"
      "  write <addr_hex> <hex_bytes> 写入内存\n"
      "  scan  <addr_hex> <len_dec> <hex_bytes>  扫描字节序列\n"
      "  watch <addr_hex> <len_dec> <interval_ms> [count]  轮询读取\n"
      "  quit | exit                  退出\n\n");
}

void PrintUsage(const char* prog) {
  std::fprintf(
      stderr,
      "memtool - 进程内存读写工具 (NDK/Linux)\n"
      "\n"
      "用法:\n"
      "  %s                      进入 UI，运行后再选择进程\n"
      "  %s <pid>                进入 UI，直接指定 pid\n"
      "  %s read  <pid> <addr_hex> <len_dec>\n"
      "  %s write <pid> <addr_hex> <hex_bytes>\n"
      "  %s ui    <pid>\n"
      "\n"
      "示例:\n"
      "  %s\n"
      "  %s 1234\n"
      "  %s read 1234 0x7ffd1234 16\n"
      "  %s write 1234 0x7ffd1234 90c3\n"
      "\n"
      "提示:\n"
      "  UI 内可用: pid/pids/maps/elfs/read/write/scan/watch\n",
      prog,
      prog,
      prog,
      prog,
      prog,
      prog,
      prog,
      prog,
      prog,
      prog);
}

void PrintRegions(const std::vector<MapRegion>& regions) {
  for (const auto& r : regions) {
    if (r.name.empty()) {
      std::printf("%" PRIxPTR "-%" PRIxPTR " %s\n", r.start, r.end,
                  r.perms.c_str());
    } else {
      std::printf("%" PRIxPTR "-%" PRIxPTR " %s %s\n", r.start, r.end,
                  r.perms.c_str(), r.name.c_str());
    }
  }
}

bool RequirePid(pid_t pid) {
  if (pid <= 0) {
    std::printf("请先用 pid <pid|package> 选择进程。\n");
    return false;
  }
  return true;
}

}  // namespace


void RunUi(pid_t pid) {
  PrintBanner();
  PrintUiHelp();
  std::string line;
  while (true) {
    if (pid > 0) {
      std::printf("memtool(pid=%d)> ", pid);
    } else {
      std::printf("memtool(%s)> ", StatusLabel(pid));
    }
    if (!std::getline(std::cin, line)) {
      break;
    }
    auto args = SplitTokens(line);
    if (args.empty()) {
      continue;
    }
    const std::string& cmd = args[0];
    if (cmd == "quit" || cmd == "exit") {
      break;
    }
    if (cmd == "help") {
      PrintUiHelp();
      continue;
    }
    if (cmd == "pid" && args.size() == 2) {
      pid_t new_pid = 0;
      if (ParsePid(args[1].c_str(), &new_pid)) {
        pid = new_pid;
        continue;
      }
      auto pids = FindPidsByPackage(args[1]);
      if (pids.empty()) {
        std::printf("未找到匹配进程。\n");
      } else {
        pid = pids[0];
        std::printf("切换到 PID: %d\n", pid);
      }
      continue;
    }
    if (cmd == "pids" && args.size() == 2) {
      auto pids = FindPidsByPackage(args[1]);
      if (pids.empty()) {
        std::printf("未找到匹配进程。\n");
      } else {
        for (pid_t p : pids) {
          std::printf("%d\n", p);
        }
      }
      continue;
    }
    if (cmd == "maps") {
      if (!RequirePid(pid)) {
        continue;
      }
      auto regions = ReadMaps(pid);
      if (regions.empty()) {
        std::perror("read maps");
      } else {
        PrintRegions(regions);
      }
      continue;
    }
    if (cmd == "elfs") {
      if (!RequirePid(pid)) {
        continue;
      }
      auto regions = ReadMaps(pid);
      if (regions.empty()) {
        std::perror("read maps");
        continue;
      }
      std::set<std::string> seen;
      for (const auto& r : regions) {
        if (r.name.empty()) {
          continue;
        }
        if (r.perms.find('x') == std::string::npos) {
          continue;
        }
        if (seen.insert(r.name).second) {
          if (IsElfFile(r.name)) {
            std::printf("%s\n", r.name.c_str());
          }
        }
      }
      continue;
    }
    if (cmd == "read" && args.size() == 3) {
      if (!RequirePid(pid)) {
        continue;
      }
      std::uintptr_t addr = 0;
      size_t len = 0;
      if (!ParseAddress(args[1].c_str(), &addr) ||
          !ParseLength(args[2].c_str(), &len)) {
        std::printf("read 参数无效。\n");
        continue;
      }
      std::vector<unsigned char> buf;
      std::string err;
      if (!ReadMemory(pid, addr, len, &buf, &err)) {
        std::printf("%s\n", err.c_str());
        continue;
      }
      PrintHex(buf);
      continue;
    }
    if (cmd == "write" && args.size() == 3) {
      if (!RequirePid(pid)) {
        continue;
      }
      std::uintptr_t addr = 0;
      if (!ParseAddress(args[1].c_str(), &addr)) {
        std::printf("地址无效。\n");
        continue;
      }
      std::vector<unsigned char> bytes;
      if (!ParseHexBytes(args[2].c_str(), &bytes)) {
        std::printf("十六进制字节无效。\n");
        continue;
      }
      std::string err;
      if (!WriteMemory(pid, addr, bytes, &err)) {
        std::printf("%s\n", err.c_str());
        continue;
      }
      std::printf("完成\n");
      continue;
    }
    if (cmd == "scan" && args.size() == 4) {
      if (!RequirePid(pid)) {
        continue;
      }
      std::uintptr_t addr = 0;
      size_t len = 0;
      if (!ParseAddress(args[1].c_str(), &addr) ||
          !ParseLength(args[2].c_str(), &len)) {
        std::printf("scan 参数无效。\n");
        continue;
      }
      std::vector<unsigned char> pattern;
      if (!ParseHexBytes(args[3].c_str(), &pattern) || pattern.empty()) {
        std::printf("十六进制字节无效。\n");
        continue;
      }
      const size_t chunk = 4096;
      std::vector<unsigned char> buf;
      std::vector<unsigned char> overlap;
      size_t offset = 0;
      while (offset < len) {
        size_t to_read = std::min(chunk, len - offset);
        std::string err;
        if (!ReadMemory(pid, addr + offset, to_read, &buf, &err)) {
          std::printf("%s\n", err.c_str());
          break;
        }
        std::vector<unsigned char> window;
        window.reserve(overlap.size() + buf.size());
        window.insert(window.end(), overlap.begin(), overlap.end());
        window.insert(window.end(), buf.begin(), buf.end());
        auto it = std::search(window.begin(), window.end(), pattern.begin(),
                              pattern.end());
        while (it != window.end()) {
          size_t match_off = static_cast<size_t>(it - window.begin());
          std::uintptr_t hit = addr + offset - overlap.size() + match_off;
          if (hit >= addr && hit + pattern.size() <= addr + len) {
            std::printf("0x%" PRIxPTR "\n", hit);
          }
          it = std::search(it + 1, window.end(), pattern.begin(),
                           pattern.end());
        }
        if (pattern.size() > 1) {
          overlap.assign(window.end() -
                             std::min(window.size(), pattern.size() - 1),
                         window.end());
        }
        offset += to_read;
      }
      continue;
    }
    if (cmd == "watch" && (args.size() == 4 || args.size() == 5)) {
      if (!RequirePid(pid)) {
        continue;
      }
      std::uintptr_t addr = 0;
      size_t len = 0;
      std::uint64_t interval = 0;
      std::uint64_t count = 0;
      if (!ParseAddress(args[1].c_str(), &addr) ||
          !ParseLength(args[2].c_str(), &len) ||
          !ParseU64(args[3], &interval)) {
        std::printf("watch 参数无效。\n");
        continue;
      }
      if (args.size() == 5 && !ParseU64(args[4], &count)) {
        std::printf("watch 次数无效。\n");
        continue;
      }
      std::uint64_t iter = 0;
      while (count == 0 || iter < count) {
        std::vector<unsigned char> buf;
        std::string err;
        if (!ReadMemory(pid, addr, len, &buf, &err)) {
          std::printf("%s\n", err.c_str());
          break;
        }
        PrintHex(buf);
        ++iter;
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
      }
      continue;
    }
    std::printf("未知或无效命令，输入 help 查看。\n");
  }
}
