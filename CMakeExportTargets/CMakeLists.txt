CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
PROJECT(CMakeExportTargets)

INCLUDE(CMakePackageConfigHelpers)
INCLUDE(GNUInstallDirs)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
LIST(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

SET(CMAKE_CXX_STANDARD 14)

ADD_LIBRARY(foo Foo.cpp)
ADD_LIBRARY(bar Foo.cpp)
ADD_LIBRARY(baz Foo.cpp)

#INSTALL(TARGETS ${PROJECT_NAME}
#        EXPORT ${PROJECT_EXPORT}
#        DESTINATION bin)
#
#INSTALL(EXPORT ${PROJECT_EXPORT}
#        FILE ${PROJECT_NAME}-targets.cmake
#        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
#)
#
#CONFIGURE_PACKAGE_CONFIG_FILE(
#        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in"
#        ${PROJECT_NAME}-config.cmake
#        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
#        NO_CHECK_REQUIRED_COMPONENTS_MACRO
#        PATH_VARS CMAKE_INSTALL_INCLUDEDIR
#)
#
#INSTALL(
#        FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
#        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
#)
SET(BIN_INSTALL_DIR bin CACHE STRING
    "The subdirectory where the compiler binary should be installed")
SET(INCLUDE_INSTALL_DIR include CACHE STRING
    "The subdirectory where include files should be installed")
SET(LIB_INSTALL_DIR lib CACHE STRING
    "The subdirectory where libraries should be installed")
SET(CMAKE_INSTALL_DIR lib/cmake/CMakeExportTargets CACHE STRING
    "The subdirectory where CMake package config files should be installed")

INSTALL(
        TARGETS
        foo
        bar
        baz
        EXPORT ${PROJECT_NAME}-exports
        DESTINATION ${LIB_INSTALL_DIR}
)
CONFIGURE_PACKAGE_CONFIG_FILE(
        cmake/CMakeExportTargets-config.cmake.in
        CMakeExportTargetsConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_DIR}
        PATH_VARS
        CMAKE_INSTALL_DIR
        BIN_INSTALL_DIR
        INCLUDE_INSTALL_DIR
)
INSTALL(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeExportTargetsConfig.cmake
        DESTINATION ${CMAKE_INSTALL_DIR}
)
INSTALL(
        EXPORT CMakeExportTargets-exports
        FILE CMakeExportTargetsTargets.cmake
        NAMESPACE CMakeExportTargets::
        DESTINATION ${CMAKE_INSTALL_DIR}
)