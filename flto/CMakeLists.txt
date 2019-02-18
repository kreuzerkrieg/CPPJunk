CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
PROJECT(flto)

SET(CMAKE_CXX_STANDARD 14)

ADD_COMPILE_OPTIONS(
        -Waligned-new
        -faligned-new
        -flto
)

ADD_LINK_OPTIONS(
#        -Wl,--verbose
        -flto)

LINK_DIRECTORIES(
        /home/ernest/Development/Reduxio/reduxio_1/ext/jemalloc/
)

INCLUDE_DIRECTORIES(
        /home/ernest/Development/jemalloc/include/jemalloc
)

#LINK_LIBRARIES(jemalloc)

ADD_EXECUTABLE(flto main.cpp)