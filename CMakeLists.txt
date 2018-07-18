cmake_minimum_required(VERSION 3.10)
project(etcdTest)

set(CMAKE_CXX_STANDARD 14)

add_executable(etcdTest
        etcd_client.cpp
        auth.pb.cc
        kv.grpc.pb.cc
        kv.pb.cc
        rpc.grpc.pb.cc
        rpc.pb.cc
        )
include_directories(/home/ernest/grpc/include)
TARGET_LINK_LIBRARIES(etcdTest
        /usr/local/lib/libprotobuf.a
        grpc++
        grpc
        grpc++_reflection
        pthread
        dl
        )