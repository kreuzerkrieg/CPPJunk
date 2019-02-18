#include <iostream>
#include <thread>
#include <cstddef>
#include <vector>
#include "Client.h"
#include "Server.h"

using namespace std::chrono_literals;

int main()
{

	std::thread serverThread([&]() {
		Server server(5454);
		server.Listen([](std::vector<std::byte>&& buffer) {
			return std::vector<std::byte>{std::byte(65), std::byte(66), std::byte(67), std::byte(68), std::byte(69), std::byte(70)};
		});
	});
	std::atomic_uint64_t count = 0;
	std::atomic_uint64_t throughput = 0;
	size_t blockSize = 512;
	int64_t cycles = 100'000;
	std::vector<std::byte> msg(blockSize, std::byte(0));

	std::vector<Client> clients(16);
	std::vector<std::thread> clientThreads;
	for (auto& client:clients) {
		client.Connect("localhost", 5454, 2s);
		client.SendMessage(msg);
	}
	auto start = std::chrono::high_resolution_clock::now();

	for (auto& client:clients) {
		clientThreads.emplace_back([&]() {
			for (auto i = 0; i < cycles; ++i) {
				throughput += blockSize;
				count += client.SendMessage(msg).size();
			}
		});
	}
	for (auto& thread:clientThreads) {
		thread.join();
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	std::cout << "Latency: " << us / cycles << "us." << std::endl;
	std::cout << "Througput: " << std::fixed << double(throughput) / us * 1'000'000 / 1024 / 1024 << "MiB/s." << std::endl;
	std::cout << "Througput: " << std::fixed << double(throughput) / blockSize / us * 1'000'000 << "messages/sec." << std::endl;
	serverThread.join();
	return 0;
}