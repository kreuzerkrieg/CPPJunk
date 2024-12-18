cmake_minimum_required(VERSION 3.22)
project(adler32)

set(CMAKE_CXX_STANDARD 23)

find_path(EVE_INCLUDE_DIRS "eve/algo/README.txt")
find_package(GTest CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

add_library(adler32 Adler32Generic.cpp Adler32Generic.h Adler32SIMD.cpp Crc32.cpp Crc32.h CRC32Intrinsic.cpp CRC32SIMD.cpp)
target_include_directories(adler32 SYSTEM PRIVATE ${EVE_INCLUDE_DIRS})
target_compile_options(adler32 PRIVATE
                       #                       -S
                       #                       -masm=intel
                       -march=native
                       -Werror
                       -Weverything
                       -Wno-global-constructors
                       -Wno-c++98-compat-pedantic
                       -Wno-c++98-compat
                       -Wno-c99-extensions
                       -Rpass=loop-vectorize
                       -Rpass-missed=loop-vectorize
                       -Rpass-analysis=loop-vectorize
                       )

add_executable(tests tests_main.cpp adler32_tests.cpp crc32_test.cpp)
target_compile_options(tests PRIVATE
                       -march=native
                       )
target_link_libraries(tests PRIVATE adler32 GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main ZLIB::ZLIB)

find_package(benchmark CONFIG REQUIRED)
add_executable(perf
               adler32_perf.cpp
               crc32_perf.cpp
               )
target_compile_options(perf PRIVATE
                       -march=native
                       )
target_link_libraries(perf PRIVATE
                      adler32
                      ZLIB::ZLIB
                      benchmark::benchmark
                      benchmark::benchmark_main
                      pthread
                      )
