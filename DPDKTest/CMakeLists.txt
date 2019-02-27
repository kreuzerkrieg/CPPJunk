CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(DPDKTest)

SET(CMAKE_CXX_STANDARD 17)
FIND_PACKAGE(dpdk REQUIRED)

ADD_EXECUTABLE(DPDKTest main.cpp)
STRING(TOLOWER ${CMAKE_BUILD_TYPE} build)
TARGET_LINK_LIBRARIES(DPDKTest PRIVATE DPDK::dpdk_${build}
                      pthread
                      numa
                      dl)
