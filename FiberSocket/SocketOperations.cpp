#include "SocketOperations.h"
#include <boost/fiber/all.hpp>

void SockOp::Send(void* buff, size_t size)
{
	auto byteBuff = reinterpret_cast<std::byte*>(buff);
	auto res = send(sock, buff, size, /*MSG_DONTWAIT*/0);
//	while ((res < size || errno == EWOULDBLOCK) && size > 0) {
//		// boost::fibers::this_fiber::yield() here
//		if (res > 0) {
//			size -= res;
//			res = send(sock, byteBuff + res, size, 0);
//		}
//		else {
//			res = send(sock, byteBuff, size, 0);
//		}
//	}
}

std::vector<std::byte> SockOp::Receive(size_t size)
{
	std::vector<std::byte> retVal;
	retVal.resize(size);
	ssize_t res;
	//while (true) {
		res = recv(sock, retVal.data(), retVal.size(),/* MSG_DONTWAIT*/0);
//		if (res < 0 && errno == EWOULDBLOCK) {
//			//boost::this_fiber::yield();
//			continue;
//		}
//		if (res == 0 || res == size) {
//			break;
//		}
//		if (res > 0) {
//			size -= res;
//			res = send(sock, retVal.data() + res, size, 0);
//		}
//		else {
//			res = send(sock, retVal.data(), size, 0);
//		}
	//}
	return retVal;
}

SockOp::SockOp(int socket) : sock(socket)
{}

void SockOp::Send(std::vector<std::byte>&& buff)
{
	Send(buff.size());
	Send(buff.data(), buff.size());
}
