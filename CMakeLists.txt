project(c64emu)
cmake_minimum_required(VERSION 3.9)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED)

file(GLOB SRC_FILES *.cpp)
add_executable(test.tsk ${SRC_FILES})
