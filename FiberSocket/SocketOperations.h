#pragma once

#include <cstddef>
#include <vector>
#include <sys/socket.h>
#include <cerrno>

class SockOp
{
public:
	SockOp() = default;

	explicit SockOp(int socket);

	template<typename T, typename std::enable_if_t<std::is_pod<T>::value>* = nullptr>
	void Send(T value)
	{
		Send(&value, sizeof(value));
	}

	template<typename T, typename std::enable_if_t<std::is_pod<T>::value>* = nullptr>
	T Receive()
	{
		T retVal;
		auto buff = reinterpret_cast<std::byte*>(&retVal);
		auto size = sizeof(T);
		auto res = recv(sock, buff, size, MSG_DONTWAIT);
		while ((res < size || errno == EWOULDBLOCK) && size > 0) {
			// boost::fibers::this_fiber::yield() here
			if (res > 0) {
				size -= res;
				res = recv(sock, buff + res, size, 0);
			}
			else {
				res = recv(sock, buff, size, 0);
			}
		}
		return retVal;
	}

	void Send(std::vector<std::byte>&& buff);

	void Send(void* buff, size_t size);

	std::vector<std::byte> Receive(size_t size);

private:
	int sock = -1;
};