cmake_minimum_required(VERSION 3.16)
project(FileStorageLevels)

set(CMAKE_CXX_STANDARD 20)
find_package(Boost REQUIRED COMPONENTS serialization)
add_executable(FileStorageLevels main.cpp ReadLeveledStorage.cpp)
target_link_libraries(FileStorageLevels PRIVATE ${Boost_LIBRARIES} pthread)
