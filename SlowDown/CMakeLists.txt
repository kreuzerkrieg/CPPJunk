cmake_minimum_required(VERSION 3.15)
project(SlowDown)

set(CMAKE_CXX_STANDARD 17)
find_package(aws-cpp-sdk-core CONFIG REQUIRED)
find_package(aws-cpp-sdk-s3 CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(SlowDown Utils.cpp FolderUploader.cpp S3MultipartUploader.cpp main.cpp)
target_link_libraries(SlowDown PRIVATE aws-cpp-sdk-s3
                      aws-cpp-sdk-core
                      OpenSSL::SSL
                      OpenSSL::Crypto
                      CURL::libcurl
                      pthread)