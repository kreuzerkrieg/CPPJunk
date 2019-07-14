cmake_minimum_required(VERSION 3.14)
project(S3Test)

set(CMAKE_CXX_STANDARD 17)

#find_package(OpenSSL REQUIRED)
#find_package(aws-cpp-sdk-core CONFIG REQUIRED)
#find_package(aws-cpp-sdk-dynamodb CONFIG REQUIRED)
#find_package(aws-cpp-sdk-kinesis CONFIG REQUIRED)
#find_package(aws-cpp-sdk-s3 CONFIG REQUIRED)

add_executable(S3Test main.cpp S3File.cpp S3LazyListRetriever.cpp S3LazyListRetriever.h)
target_link_libraries(S3Test PRIVATE
#                      ${CMAKE_DL_LIBS}
#                      OpenSSL::SSL
#                      OpenSSL::Crypto
                      aws-cpp-sdk-s3
                      aws-cpp-sdk-core
#                      aws-cpp-sdk-dynamodb
#                      aws-cpp-sdk-kinesis
                      stdc++fs
                      pthread
                      dl
                      )

#target_link_libraries(S3Test PUBLIC
#                      ${CMAKE_DL_LIBS}
#                      OpenSSL::SSL
#                      OpenSSL::Crypto
#                      pthread
#                      dl
#                      aws-cpp-sdk-core
#                      aws-cpp-sdk-dynamodb
#                      aws-cpp-sdk-kinesis
#                      aws-cpp-sdk-s3
#                      dl
#                      )
