cmake_minimum_required(VERSION 3.14)
project(S3Test)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED)
add_executable(S3Test main.cpp S3File.cpp S3LazyListRetriever.cpp Fs.cpp Downloader.cpp Uploader.cpp)
target_link_libraries(S3Test PRIVATE
                      aws-cpp-sdk-s3
                      aws-cpp-sdk-core
                      ${Boost_LIBRARIES}
                      stdc++fs
                      pthread
                      dl
                      )
target_include_directories(S3Test PRIVATE ${Boost_INCLUDE_DIRS})