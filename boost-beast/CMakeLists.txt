CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
PROJECT(boost_beast)

SET(CMAKE_CXX_STANDARD 17)

FIND_PACKAGE(Boost REQUIRED COMPONENTS
             thread
             system
             coroutine
             )

LINK_LIBRARIES(
        ${Boost_LIBRARIES}
        pthread
)

ADD_EXECUTABLE(client websocket_client_coro.cpp)
ADD_EXECUTABLE(server websocket_server_stackless.cpp)
