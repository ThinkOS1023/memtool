#include "memtool_memory.h"

#include <cerrno>
#include <cstdio>
#include <cstring>

#include <fcntl.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>

namespace {

bool AttachProcess(pid_t pid) {
  if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) == -1) {
    return false;
  }
  int status = 0;
  if (waitpid(pid, &status, 0) == -1) {
    return false;
  }
  return WIFSTOPPED(status) != 0;
}

void DetachProcess(pid_t pid) {
  ptrace(PTRACE_DETACH, pid, nullptr, nullptr);
}

ssize_t ReadViaProcessVm(pid_t pid, std::uintptr_t addr, void* buf, size_t len) {
#if defined(SYS_process_vm_readv)
  iovec local = {};
  iovec remote = {};
  local.iov_base = buf;
  local.iov_len = len;
  remote.iov_base = reinterpret_cast<void*>(addr);
  remote.iov_len = len;
  return syscall(SYS_process_vm_readv, pid, &local, 1, &remote, 1, 0);
#else
  (void)pid;
  (void)addr;
  (void)buf;
  (void)len;
  errno = ENOSYS;
  return -1;
#endif
}

ssize_t WriteViaProcessVm(pid_t pid, std::uintptr_t addr, const void* buf,
                          size_t len) {
#if defined(SYS_process_vm_writev)
  iovec local = {};
  iovec remote = {};
  local.iov_base = const_cast<void*>(buf);
  local.iov_len = len;
  remote.iov_base = reinterpret_cast<void*>(addr);
  remote.iov_len = len;
  return syscall(SYS_process_vm_writev, pid, &local, 1, &remote, 1, 0);
#else
  (void)pid;
  (void)addr;
  (void)buf;
  (void)len;
  errno = ENOSYS;
  return -1;
#endif
}

int OpenMemFd(pid_t pid) {
  char path[64];
  std::snprintf(path, sizeof(path), "/proc/%d/mem", pid);
  return open(path, O_RDWR | O_CLOEXEC);
}

bool ReadViaProcMem(pid_t pid, std::uintptr_t addr, void* buf, size_t len) {
  int fd = OpenMemFd(pid);
  if (fd == -1) {
    return false;
  }
  ssize_t got = pread(fd, buf, len, static_cast<off_t>(addr));
  close(fd);
  return got == static_cast<ssize_t>(len);
}

bool WriteViaProcMem(pid_t pid, std::uintptr_t addr, const void* buf,
                     size_t len) {
  int fd = OpenMemFd(pid);
  if (fd == -1) {
    return false;
  }
  ssize_t wrote = pwrite(fd, buf, len, static_cast<off_t>(addr));
  close(fd);
  return wrote == static_cast<ssize_t>(len);
}

}  // namespace

bool ReadMemory(pid_t pid, std::uintptr_t addr, size_t len,
                std::vector<unsigned char>* out, std::string* err) {
  out->assign(len, 0);
  if (!AttachProcess(pid)) {
    if (err) {
      *err = std::string("ptrace attach failed: ") + std::strerror(errno);
    }
    return false;
  }
  bool ok = false;
  ssize_t got = ReadViaProcessVm(pid, addr, out->data(), len);
  if (got == static_cast<ssize_t>(len)) {
    ok = true;
  } else if (ReadViaProcMem(pid, addr, out->data(), len)) {
    ok = true;
  } else if (err) {
    *err = std::string("read memory failed: ") + std::strerror(errno);
  }
  DetachProcess(pid);
  return ok;
}

bool WriteMemory(pid_t pid, std::uintptr_t addr,
                 const std::vector<unsigned char>& bytes, std::string* err) {
  if (!AttachProcess(pid)) {
    if (err) {
      *err = std::string("ptrace attach failed: ") + std::strerror(errno);
    }
    return false;
  }
  bool ok = false;
  ssize_t wrote = WriteViaProcessVm(pid, addr, bytes.data(), bytes.size());
  if (wrote == static_cast<ssize_t>(bytes.size())) {
    ok = true;
  } else if (WriteViaProcMem(pid, addr, bytes.data(), bytes.size())) {
    ok = true;
  } else if (err) {
    *err = std::string("write memory failed: ") + std::strerror(errno);
  }
  DetachProcess(pid);
  return ok;
}
