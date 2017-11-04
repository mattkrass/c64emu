project(c64emu)
cmake_minimum_required(VERSION 3.9)
enable_testing()

file(GLOB SRC_FILES *.cpp)
add_executable(test.tsk ${SRC_FILES})
