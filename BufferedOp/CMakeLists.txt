CMAKE_MINIMUM_REQUIRED(VERSION 3.12)
PROJECT(BufferedOp)

SET(CMAKE_CXX_STANDARD 17)

ADD_EXECUTABLE(BufferedOp main.cpp)
TARGET_COMPILE_OPTIONS(BufferedOp PUBLIC
                       --coverage)
TARGET_LINK_LIBRARIES(BufferedOp
                      gcov
                      )