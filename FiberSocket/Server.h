#pragma once

#include <cstdint>
#include <netinet/in.h>
#include <stdexcept>
#include <libexplain/socket.h>
#include <chrono>
#include "SocketOperations.h"
#include <netinet/tcp.h>
#include "Client.h"
#include "ErrorHandling.h"

class Server
{
public:
	explicit Server(uint16_t port);

	~Server();

	template<typename T>
	void Listen(T&& functor)
	{
		sockaddr_in cli_addr = {0};
		socklen_t clilen = sizeof(cli_addr);
		while (!shouldStop) {
			auto datasock = accept(sock, (struct sockaddr*) &cli_addr, &clilen);
			if (datasock < 0) {
				throw std::runtime_error(explain_socket(AF_INET, SOCK_STREAM, 0));
			}
			timeval tv = {};
			tv.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(2s).count();
			CheckErrno(setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(timeval)));
			CheckErrno(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(timeval)));
			int optval = 1;
			CheckErrno(setsockopt(datasock, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)));
			// launch fiber here
			SockOp operation(datasock);
			while (!shouldStop) { //fiber body
				auto size = operation.Receive<size_t>();
				operation.Send(functor(operation.Receive(size)));
			}
		}
	}

	void Stop();

private:
	bool shouldStop = false;
	int sock = -1;
};