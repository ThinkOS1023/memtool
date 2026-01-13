cmake_minimum_required(VERSION 3.10)
project(ndk_demo CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(memtool
  tools/memtool_main.cpp
  tools/memtool_ui.cpp
  tools/memtool_proc.cpp
  tools/memtool_memory.cpp
  tools/memtool_util.cpp
)
