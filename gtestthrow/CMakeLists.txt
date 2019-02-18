CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
PROJECT(gtestthrow)

SET(CMAKE_CXX_STANDARD 17)
FIND_PACKAGE(GTest REQUIRED)
ADD_EXECUTABLE(gtestthrow main.cpp)
TARGET_LINK_LIBRARIES(gtestthrow PUBLIC
                      GTest::GTest
                      GTest::Main
                      )