#pragma once

#include <cstdint>
#include <chrono>

int InitServerSocket(uint16_t port);

int InitClientSocket(const char* remoteHost, uint16_t port, std::chrono::microseconds timeout);
