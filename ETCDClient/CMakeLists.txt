CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(etcdTest)

SET(CMAKE_CXX_STANDARD 14)
FIND_PACKAGE(Protobuf REQUIRED)
FIND_PACKAGE(GRPC REQUIRED)
INCLUDE(${CMAKE_SOURCE_DIR}/cmake/FindGRPC.cmake)

SET(ProtoFiles
    ${CMAKE_SOURCE_DIR}/proto/auth.proto
    ${CMAKE_SOURCE_DIR}/proto/etcdserver.proto
    ${CMAKE_SOURCE_DIR}/proto/kv.proto
    ${CMAKE_SOURCE_DIR}/proto/rpc.proto
    )

SET(ETCDIncludePath ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "Path to generated protobuf files.")
PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders ${ProtoFiles})
GRPC_GENERATE_CPP(GRPCSources GRPCHeaders ${CMAKE_CURRENT_BINARY_DIR} ${ProtoFiles})

ADD_EXECUTABLE(etcdTest etcd_client.cpp
               ${ProtoSources} ${ProtoHeaders} ${GRPCSources} ${GRPCHeaders}
               )

TARGET_INCLUDE_DIRECTORIES(etcdTest PRIVATE
                           ${GRPC_INCLUDE_DIR}
                           ${PROTOBUF_INCLUDE_DIR}
                           ${ETCDIncludePath}
                           )

TARGET_LINK_LIBRARIES(etcdTest
                      ${PROTOBUF_LIBRARY}
                      gRPC::grpc
                      gRPC::grpc++
                      pthread
                      dl
                      )