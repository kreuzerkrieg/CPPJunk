cmake_minimum_required(VERSION 3.14)
project(SearchAndReplace)

set(CMAKE_CXX_STANDARD 17)

add_executable(SearchAndReplace main.cpp X3.cpp BoostSpirit.cpp BoostAlgoString.cpp Manual.cpp)

find_package(Boost REQUIRED)
target_link_libraries(SearchAndReplace PRIVATE ${Boost_LIBRARIES} pthread)
target_include_directories(SearchAndReplace PRIVATE ${Boost_INCLUDE_DIRS})