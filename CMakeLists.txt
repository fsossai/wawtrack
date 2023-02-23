cmake_minimum_required(VERSION 3.13)

project(wawtrack)
enable_language(CXX)

find_package(LLVM REQUIRED CONFIG)

add_definitions(${LLVM_DEFINITIONS})

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

configure_file(wawtrack++.in wawtrack++ @ONLY)

add_subdirectory(lib)
add_subdirectory(pass)
