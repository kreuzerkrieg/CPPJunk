#include <netinet/in.h>
#include <netdb.h>
#include <cstring>
#include <stdexcept>
#include <libexplain/socket.h>
#include <netinet/tcp.h>
#include "SocketSetup.h"
#include "ErrorHandling.h"

int InitServerSocket(uint16_t port)
{
	sockaddr_in serv_addr = {0};

	auto sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		throw std::runtime_error(explain_socket(AF_INET, SOCK_STREAM, 0));
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	CheckErrno(bind(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)));
	CheckErrno(listen(sock, 512));

	return sock;
}

int InitClientSocket(const char* remoteHost, uint16_t port, std::chrono::microseconds timeout)
{
	auto sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		throw std::runtime_error(explain_socket(AF_INET, SOCK_STREAM, 0));
	}
	union TargetAddress
	{
		sockaddr sa;
		sockaddr_in in;
	};
	TargetAddress targetAddress = {};
	targetAddress.in.sin_family = AF_INET;
	targetAddress.in.sin_port = htons(port);

	hostent* hp = gethostbyname(remoteHost);
	memcpy(&targetAddress.in.sin_addr, hp->h_addr_list[0], sizeof(targetAddress.in.sin_addr));

	timeval tv = {};
	tv.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(timeout).count();
	CheckErrno(setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(timeval)));
	CheckErrno(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(timeval)));
	CheckErrno(connect(sock, &targetAddress.sa, sizeof(targetAddress)));

	int optval = 1;
	CheckErrno(setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)));
	return sock;
}