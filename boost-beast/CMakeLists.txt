CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
PROJECT(boost_beast)

SET(CMAKE_CXX_STANDARD 14)

FIND_PACKAGE(Boost REQUIRED COMPONENTS
             coroutine
             system
             thread
             )
ADD_COMPILE_DEFINITIONS(
        BOOST_COROUTINES_NO_DEPRECATION_WARNING
        BOOST_SYSTEM_NO_DEPRECATED
        BOOST_ERROR_CODE_HEADER_ONLY
)

ADD_EXECUTABLE(client websocket_client_coro.cpp)
TARGET_LINK_LIBRARIES(client PRIVATE
                      ${Boost_LIBRARIES}
                      pthread
                      rt
                      )
ADD_EXECUTABLE(server websocket_server_stackless.cpp)
TARGET_LINK_LIBRARIES(server PRIVATE
                      ${Boost_LIBRARIES}
                      pthread
                      )