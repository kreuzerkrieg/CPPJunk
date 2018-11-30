#include "Server.h"
#include "SocketSetup.h"

Server::Server(uint16_t port)
{
	sock = InitServerSocket(port);
}

Server::~Server()
{
	shouldStop = true;
	shutdown(sock, SHUT_RDWR);
}

void Server::Stop()
{
	shouldStop = true;
}