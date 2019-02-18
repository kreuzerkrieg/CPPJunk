CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
PROJECT(NVMeTest)

SET(CMAKE_CXX_STANDARD 17)

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/artifacts/${CMAKE_BUILD_TYPE}/lib")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/artifacts/${CMAKE_BUILD_TYPE}/lib")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/artifacts/${CMAKE_BUILD_TYPE}/bin")

LINK_DIRECTORIES(
        /home/ernest/spdk/build/lib
        /home/ernest/spdk/dpdk/build/lib
)

INCLUDE_DIRECTORIES(
        /home/ernest/spdk/include
)

#LINK_DIRECTORIES(
#        /home/ernest/Development/Reduxio/reduxio/ext/spdk/build/lib
#        /home/ernest/Development/Reduxio/reduxio/ext/spdk/dpdk/build/lib
#)
#
#INCLUDE_DIRECTORIES(
#        /home/ernest/Development/Reduxio/reduxio/ext/spdk/include
#)

IF (CMAKE_BUILD_TYPE STREQUAL Debug)
    LINK_DIRECTORIES(
            /home/ernest/Development/Reduxio/reduxio/ext/jemalloc/Debug
    )
ELSE ()
    LINK_DIRECTORIES(
            /home/ernest/Development/Reduxio/reduxio/ext/jemalloc/Release
    )
ENDIF ()

LINK_LIBRARIES(jemalloc)

ADD_EXECUTABLE(NVMeTest main.cpp NVMeManager.cpp)

SET(DPDK_LIBS
    dpdk
    )

SET(SPDK_LIBS
    spdk_app_rpc
    spdk_bdev
    spdk_bdev_null
    spdk_bdev_rpc
    spdk_conf
    spdk_copy
    spdk_thread
    spdk_event
    spdk_event_bdev
    spdk_event_copy
    spdk_event_net
    spdk_net
    spdk_nvmf
    spdk_nvme
    spdk_sock_posix
    spdk_sock
    spdk_rpc
    spdk_jsonrpc
    spdk_json
    spdk_trace
    spdk_util
    spdk_log
    spdk_env_dpdk
    )

TARGET_LINK_LIBRARIES(NVMeTest
                      ${SPDK_LIBS}
                      ${DPDK_LIBS}
                      ibverbs
                      rdmacm
                      pthread
                      dl
                      numa
                      uuid
                      rt
                      )
