#include <stdexcept>
#include <cstring>
#include <chrono>
#include "Client.h"
#include "SocketSetup.h"

Client::Client()
{
}

Client::~Client()
{
	int rc = shutdown(sock, SHUT_RDWR);
	if (rc < 0 && errno == ENOTCONN) {
		return;
	}
}

void Client::Connect(const char* remoteHost, uint16_t port, std::chrono::microseconds timeout)
{
	sock = InitClientSocket(remoteHost, port, timeout);
	operation = SockOp(sock);
}

std::vector<std::byte> Client::SendMessage(std::vector<std::byte>& buff)
{

	operation.Send(buff.size());
	operation.Send(buff.data(), buff.size());
	auto responseSize = operation.Receive<size_t>();
	return operation.Receive(responseSize);
}

