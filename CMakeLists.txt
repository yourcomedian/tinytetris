# CMakeLists.txt for TinyTetrisCore library
# 这个文件定义了如何编译俄罗斯方块游戏的C++核心库
# CMake是一个跨平台的构建系统，能够生成各种平台的Makefile或工程文件

# 设置最低要求的CMake版本
cmake_minimum_required(VERSION 3.10)

# 定义项目名称和语言
project(TinyTetrisCore CXX)

# 设置C++标准
# C++11提供了许多现代C++特性，如auto关键字、lambda表达式等
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)  # 要求编译器必须支持C++11
set(CMAKE_CXX_EXTENSIONS OFF) # 禁用编译器特定的扩展，确保兼容标准C++

# 启用位置无关代码（Position Independent Code）
# 这对于构建共享库（动态链接库）是必需的
# 相当于gcc/g++的-fPIC编译选项
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 定义库的源文件
# 这里只有一个源文件：tetris_game.cpp
set(LIB_SOURCES tetris_game.cpp)

# 添加共享库（动态链接库）目标
# 根据不同的操作系统，会生成不同的文件：
# - Linux: libtetris_core.so
# - macOS: libtetris_core.dylib
# - Windows: tetris_core.dll
# CMake会自动处理库名称前缀和扩展名
add_library(tetris_core SHARED ${LIB_SOURCES})

# 添加编译器警告选项
# 只在使用GCC或Clang编译器时添加这些选项
# -Wall: 启用大多数警告
# -Wextra: 启用额外的警告
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(tetris_core PRIVATE -Wall -Wextra)
endif()

# 关于优化的说明：
# CMake的Release构建类型默认包含优化（通常是-O2或-O3）
# 可以通过以下方式显式设置优化级别（只在Release模式生效）：
# target_compile_options(tetris_core PRIVATE "$<$<CONFIG:Release>:-O2>")

# 补充说明：
# 1. tetris_game.h文件被tetris_game.cpp隐式包含，CMake会自动找到它
# 2. 使用add_library(SHARED ...)时，CMake会自动处理特定操作系统的链接器选项
#    不需要像Makefile中那样显式设置-shared选项

# 安装规则（可选但推荐）
# 如果需要安装库和头文件到系统路径，取消下面的注释
# install(TARGETS tetris_core DESTINATION lib)
# install(FILES tetris_game.h DESTINATION include) 