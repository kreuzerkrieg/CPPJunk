cmake_minimum_required(VERSION 3.10)
project(S3Test)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED)
add_executable(S3Test main.cpp
               Utils.cpp
               S3File.cpp
               S3DirectoryIterator.cpp
               S3LazyListRetriever.cpp
               Fs.cpp
               DownloaderV2.cpp
               Uploader.cpp
               )
target_link_libraries(S3Test PRIVATE
                      aws-cpp-sdk-s3
                      aws-cpp-sdk-core
                      aws-c-event-stream
                      aws-checksums
                      aws-c-common
                      stdc++fs
                      ssl
                      crypto
                      curl
                      pthread
                      )
target_include_directories(S3Test PRIVATE ${Boost_INCLUDE_DIRS})