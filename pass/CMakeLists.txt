find_package(LLVM REQUIRED CONFIG)

set(src_files "WAWTrack.cpp")
set_source_files_properties(${src_files} PROPERTIES COMPILE_FLAGS " -std=c++17")

add_llvm_library("WAWTrack" MODULE ${src_files})
