#import "SocketOperations.h"

void SockOp::Send(void* buff, size_t size)
{
	auto byteBuff = reinterpret_cast<std::byte*>(buff);
	auto res = send(sock, buff, size, 0);
	while (res < size || res == EWOULDBLOCK) {
		// boost::fibers::this_fiber::yield() here
		if (res > 0) {
			size -= res;
			res = send(sock, byteBuff + res, size, 0);
		}
		else {
			res = send(sock, byteBuff, size, 0);
		}
	}
}

std::vector<std::byte> SockOp::Receive(size_t size)
{
	std::vector<std::byte> retVal;
	retVal.resize(size);
	auto res = recv(sock, retVal.data(), retVal.size(), 0);
	while (res < size || res == EWOULDBLOCK) {
		// boost::fibers::this_fiber::yield() here
		if (res > 0) {
			size -= res;
			res = send(sock, retVal.data() + res, size, 0);
		}
		else {
			res = send(sock, retVal.data(), size, 0);
		}
	}
	return retVal;
}

SockOp::SockOp(int socket) : sock(socket)
{}

void SockOp::Send(std::vector<std::byte>&& buff)
{
	Send(buff.size());
	Send(buff.data(), buff.size());
}
