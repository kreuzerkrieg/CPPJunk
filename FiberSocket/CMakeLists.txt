CMAKE_MINIMUM_REQUIRED(VERSION 3.12)
PROJECT(FiberSocket)

SET(CMAKE_CXX_STANDARD 17)

ADD_EXECUTABLE(FiberSocket main.cpp Client.cpp SocketSetup.cpp SocketSetup.h SocketOperations.cpp SocketOperations.h Server.cpp Server.h ErrorHandling.cpp ErrorHandling.h)
TARGET_LINK_LIBRARIES(FiberSocket PRIVATE
                      explain
                      pthread)