# memtool

面向 Linux/Android (NDK) 的跨进程内存读写工具，支持 CLI 子命令与交互式 UI。
适合在本地设备上快速定位进程内存、读取/写入字节、扫描模式和查看加载的 ELF。

## 功能概览
- 读写指定进程内存（`read/write`）
- UI 交互模式（`memtool` 或 `memtool <pid>`）
- 解析 `/proc/<pid>/maps` 与加载的 ELF 列表（`maps/elfs`）
- 扫描字节序列（`scan`）与轮询读取（`watch`）
- 包名→PID 查询（`pid/pids`）

## 构建

### Linux 本地构建
```bash
cmake -S . -B build
cmake --build build
```

### Android NDK (CMake Toolchain)
```bash
export ANDROID_NDK=~/android-ndk-r29
./build-android.sh
```

可选环境变量：
- `ANDROID_ABI`（默认 `arm64-v8a`）
- `ANDROID_PLATFORM`（默认 `21`）
- `BUILD_DIR`（默认 `build-android`）

## 使用

### 交互式 UI
```bash
./memtool
```
进入后执行：
```text
pid <pid|package>
maps
elfs
read  0x7ffd1234 16
write 0x7ffd1234 90c3
scan  0x7ffd1000 4096 90c3
watch 0x7ffd1234 16 500 10
```

### 子命令
```bash
./memtool read 1234 0x7ffd1234 16
./memtool write 1234 0x7ffd1234 90c3
```

## 权限说明
读取或写入其它进程内存通常需要 root 或与目标进程相同 UID 的权限。
Android 设备上还可能受到 SELinux/Yama 限制。

## 目录结构
- `tools/`：工具源码（模块化 C++ 实现）
- `CMakeLists.txt`：构建入口
- `build-android.sh`：NDK CMake 构建脚本
