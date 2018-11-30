#pragma once

#include "SocketOperations.h"

using namespace std::chrono_literals;

void CheckErrno(int err);

class Client
{
public:
	Client();

	~Client();

	void Connect(const char* remoteHost, uint16_t port, std::chrono::microseconds timeout);

	std::vector<std::byte> SendMessage(std::vector<std::byte>& buff);

private:
	SockOp operation;
	int sock = -1;
};
