CMAKE_MINIMUM_REQUIRED(VERSION 3.12)
PROJECT(AsyncWrapper)

SET(CMAKE_CXX_STANDARD 17)

ADD_EXECUTABLE(AsyncWrapper main.cpp)
ADD_COMPILE_OPTIONS(--coverage)
TARGET_LINK_LIBRARIES(AsyncWrapper
                      pthread
                      gcov
                      )