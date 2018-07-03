CMAKE_MINIMUM_REQUIRED(VERSION 3.9)
PROJECT(spdk_test)

SET(CMAKE_CXX_STANDARD 17)

INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_SOURCE_DIR}/spdk/include
                    ${CMAKE_SOURCE_DIR}/nonius
                    )
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/spdk/build/lib
                 ${CMAKE_SOURCE_DIR}/spdk/dpdk/build/lib/
                 /usr/lib/x86_64-linux-gnu/)

ADD_COMPILE_OPTIONS(-Wall
                    -Wpedantic
                    -fPIC
                    )

#IF (CMAKE_BUILD_TYPE EQUAL "RELEASE")
ADD_COMPILE_OPTIONS(
#        -fno-omit-frame-pointer
#        -O3
)
#ENDIF ()
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")

FIND_LIBRARY(SYSLIBS NAMES
             rt
             uuid
             numa
             rdmacm
             )
ADD_EXECUTABLE(spdk main.cpp NVMeStriper.cpp NVMeStriper.h NVMeManager.cpp NVMeManager.h Throughput.cpp Throughput.h)
SET_TARGET_PROPERTIES(spdk
                      PROPERTIES
                      ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
                      )
TARGET_LINK_LIBRARIES(spdk

                      libspdk_app_rpc.a
                      libspdk_bdev.a
                      libspdk_bdev_aio.a
                      libspdk_bdev_malloc.a
                      libspdk_bdev_null.a
                      libspdk_bdev_nvme.a
                      libspdk_bdev_rpc.a
                      libspdk_bdev_virtio.a
                      libspdk_blob.a
                      libspdk_blob_bdev.a
                      libspdk_blobfs.a
                      libspdk_conf.a
                      libspdk_copy.a
                      libspdk_copy_ioat.a
                      libspdk_cunit.a

                      libspdk_event.a
                      libspdk_event_bdev.a
                      libspdk_event_copy.a
                      libspdk_event_iscsi.a
                      libspdk_event_nbd.a
                      libspdk_event_net.a
                      libspdk_event_scsi.a
                      libspdk_event_vhost.a
                      libspdk_ioat.a
                      libspdk_iscsi.a
                      libspdk_json.a
                      libspdk_jsonrpc.a

                      libspdk_log_rpc.a
                      libspdk_lvol.a
                      libspdk_nbd.a
                      libspdk_net.a
                      libspdk_nvme.a
                      libspdk_nvmf.a
                      libspdk_rpc.a
                      libspdk_rte_vhost.a
                      libspdk_scsi.a
                      libspdk_spdk_mock.a
                      libspdk_trace.a
                      libspdk_util.a
                      libspdk_vbdev_error.a
                      libspdk_vbdev_gpt.a
                      libspdk_vbdev_lvol.a
                      libspdk_vbdev_split.a
                      libspdk_vhost.a
                      libspdk_virtio.a
                      libspdk_log.a
                      libspdk_env_dpdk.a
                      libdpdk.a
                      rdmacm
                      libnuma.a
                      dl
                      ibverbs
                      ${SYSLIBS}
                      uuid
                      rt
                      )