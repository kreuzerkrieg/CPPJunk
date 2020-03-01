cmake_minimum_required(VERSION 3.16)
project(StringJoin)

set(CMAKE_CXX_STANDARD 20)

add_executable(StringJoin main.cpp)
target_link_libraries(StringJoin pthread)

if (NOT CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(StringJoin PRIVATE -O3 -march=native)
endif ()