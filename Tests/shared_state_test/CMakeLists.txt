CMAKE_MINIMUM_REQUIRED(VERSION 3.9)
PROJECT(shared_state_test)

SET(CMAKE_CXX_STANDARD 17)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/../nonius/ /home/ernest/Downloads/boost_1_66_0)
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/bin)

ADD_EXECUTABLE(shared_state_test main.cpp Throughput.h)
SET_TARGET_PROPERTIES(shared_state_test
                      PROPERTIES
                      ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
                      )
TARGET_LINK_LIBRARIES(shared_state_test
                      boost_system
                      boost_filesystem
                      boost_context
                      boost_thread
                      boost_fiber
                      pthread
                      )