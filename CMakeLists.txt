project(c64emu)
cmake_minimum_required(VERSION 3.9)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories($ENV{PROJDIR}/libsdl-dev/include/SDL2)
link_directories($ENV{PROJDIR}/libsdl-dev/lib)

file(GLOB SRC_FILES *.cpp)
add_executable(test.tsk ${SRC_FILES})
target_link_libraries(test.tsk SDL2)
