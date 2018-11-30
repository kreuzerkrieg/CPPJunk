#include <iostream>
#include <thread>
#include <cstddef>
#include <vector>
#include "Client.h"
#include "Server.h"

using namespace std::chrono_literals;

int main()
{
	Server server(5454);
	std::thread serverThread([&]() {
		server.Listen([](std::vector<std::byte>&& buffer) {
			return std::vector<std::byte>{std::byte(65), std::byte(66), std::byte(67), std::byte(68), std::byte(69), std::byte(70)};
		});
	});

	Client client;
	client.Connect("localhost", 5454, 2s);
	uint64_t count = 0;
	uint64_t throughput = 0;
	size_t blockSize = 512;
	int64_t cycles = 1'000;
	std::vector<std::byte> msg(blockSize, std::byte(0));

	client.SendMessage(msg);
	client.SendMessage(msg);

	auto start = std::chrono::high_resolution_clock::now();
	for (auto i = 0; i < cycles; ++i) {
		throughput += blockSize;
		count += client.SendMessage(msg).size();
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	std::cout << "Latency: " << us / cycles << "us." << std::endl;
	std::cout << "Througput: " << std::fixed << double(throughput) / us * 1'000'000 / 1024 / 1024 << "MiB/s." << std::endl;
	server.Stop();
	serverThread.join();
	return 0;
}