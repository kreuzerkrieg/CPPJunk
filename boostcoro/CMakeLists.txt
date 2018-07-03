cmake_minimum_required(VERSION 3.10)
project(boostcoro)

set(CMAKE_CXX_STANDARD 14)

add_executable(boostcoro main.cpp)

add_definitions(-DBOOST_COROUTINES_NO_DEPRECATION_WARNING)
TARGET_LINK_LIBRARIES(
        boostcoro
        rt
        boost_coroutine
        boost_system
#        boost_filesystem
        boost_context
#        boost_thread
        boost_fiber
        pthread

        )