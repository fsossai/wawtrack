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

configure_file(wopt.in wopt @ONLY)
configure_file(wawtrack++.in wawtrack++ @ONLY)
configure_file(enable.in enable @ONLY)

add_subdirectory(runtime)
add_subdirectory(pass)
