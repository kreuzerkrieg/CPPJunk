#include <iostream>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <thread>
#include "ZConfig.h"
#include "Client.h"

int main(int argc, char** argv)
{
	zmq::context_t ctx;
	/*{
		MQConfig clientConfig(ctx);
		clientConfig.uri = "tcp://127.0.0.1:55556";
		MClient client(std::move(clientConfig));
		std::this_thread::sleep_for(std::chrono::seconds(15));
		boost::uuids::random_generator gen;
		boost::uuids::uuid id = gen();
		auto clientId = boost::uuids::to_string(id);
		std::cout << "Client ID: " << clientId << std::endl;
		std::string str = "Hello";
		str += boost::uuids::to_string(id);
		str.resize(1024, 'a');
		auto cycles = 10'000ul;
		auto start = std::chrono::high_resolution_clock::now();
		double throughput = 0;

		auto msg = str + "_" + clientId;
		const size_t ioDepth = 1;
		for (auto i = 0ul; i < cycles; ++i) {
			std::vector<std::pair<std::future<void>, Message>> futures;
			futures.reserve(ioDepth);
			for (auto j = 0ul; j < ioDepth; ++j) {
				throughput += msg.size();
				try {
					futures.emplace_back(client.sendRequest(msg));
					//std::cout << std::string(reinterpret_cast<char*>(reply.data()), reply.size()) << std::endl;
				}
				catch (const std::exception& ex) {
					std::cout << "Client failed. Reason: " << ex.what() << std::endl;
				}
			}
			for (auto& fut:futures) {
				fut.first.get();
				fut.second.get();
			}
		}
		auto end = std::chrono::high_resolution_clock::now();
		auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		std::cout << "Latency: " << us / (cycles * ioDepth) << "us." << std::endl;
		std::cout << "Througput: " << std::fixed << throughput / us * 1'000'000 / 1024 / 1024 << "MiB/s." << std::endl;
	}*/
	{
		bool shouldStart = false;
		std::atomic_uint64_t throughput{0};
		std::vector<std::thread> pool;
		auto cycles = 1'000ul;
		for (auto i:{1, 2, 3, 4, 5, 6, 7, 8}) {
			pool.emplace_back(std::thread([&]() {
				MQConfig clientConfig(ctx);
				clientConfig.uri = "tcp://127.0.0.1:55556";
				MClient client(std::move(clientConfig));
				//				std::this_thread::sleep_for(std::chrono::seconds(15));
				boost::uuids::random_generator gen;
				boost::uuids::uuid id = gen();
				auto clientId = boost::uuids::to_string(id);
				std::cout << "Client ID: " << clientId << std::endl;
				std::string str = "Hello";
				str += boost::uuids::to_string(id);
				str.resize(1024, 'a');
				while (!shouldStart) {
					std::this_thread::yield();
				}
				auto msg = str + "_" + clientId;
				const size_t ioDepth = 1;
				for (auto i = 0ul; i < cycles; ++i) {
					std::vector<std::pair<std::future<void>, Message>> futures;
					futures.reserve(ioDepth);
					for (auto j = 0ul; j < ioDepth; ++j) {
						throughput += msg.size();
						try {
							futures.emplace_back(client.sendRequest(msg));
							//std::cout << std::string(reinterpret_cast<char*>(reply.data()), reply.size()) << std::endl;
						}
						catch (const std::exception& ex) {
							std::cout << "Client failed. Reason: " << ex.what() << std::endl;
						}
					}
					for (auto& fut:futures) {
						fut.first.get();
						fut.second.get();
					}
				}
			}));
		}
		auto start = std::chrono::high_resolution_clock::now();
		shouldStart = true;
		for (auto& thread:pool) {
			thread.join();
		}
		auto end = std::chrono::high_resolution_clock::now();
		auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		std::cout << "Latency: " << us / (cycles * pool.size()) << "us." << std::endl;
		std::cout << "Througput: " << std::fixed << throughput / us * 1'000'000 / 1024 / 1024 << "MiB/s." << std::endl;
	}
	/*{
		std::vector<std::unique_ptr<MClient>> clients;
		for (auto i:{1, 2}) {
			MQConfig clientConfig(ctx);
			clientConfig.uri = "tcp://127.0.0.1:55556";
			clients.emplace_back(std::make_unique<MClient>(std::move(clientConfig)));
		}

		boost::uuids::random_generator gen;
		boost::uuids::uuid id = gen();
		auto clientId = boost::uuids::to_string(id);
		std::cout << "Client ID: " << clientId << std::endl;
		std::string str = "Hello";
		str.resize(1024, 'a');
		auto cycles = 100ul;
		auto start = std::chrono::high_resolution_clock::now();
		double throughput = 0;

		auto poolSize = 10;

		for (auto j = 0ul; j < cycles / poolSize; ++j) {
			std::vector<std::pair<std::future<void>, Message>> pool;
			for (auto i = 0ul; i < poolSize; ++i) {
				auto msg = str + "_" + clientId;
				throughput += msg.size();
				try {
					pool.emplace_back(clients[i % clients.size()]->sendRequest(msg));
				}
				catch (const std::exception& ex) {
					std::cout << "Client failed. Reason: " << ex.what() << std::endl;
				}
			}
			for (auto& message:pool) {
				if (message.second.valid()) {
					try {
						message.first.get();
						message.second.get();
					}
					catch (std::exception& ex) {
						std::cout << "Future exception: " << ex.what() << std::endl;
					}
				}
				else {
					std::cout << "Invalid future." << std::endl;
				}
			}
		}
		auto end = std::chrono::high_resolution_clock::now();
		auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		std::cout << "Async client latency: " << us / cycles << "us." << std::endl;
		std::cout << "Througput: " << std::fixed << throughput / us * 1'000'000 / 1024 / 1024 << "MiB/s." << std::endl;
	}*/

	/*{
		bool shouldStart = false;
		auto numberOfClients = 32ul;
		std::vector<std::thread> threads;
		std::atomic<uint64_t> bytes{0};

		MQConfig clientConfig(ctx);
		clientConfig.uri = "tcp://127.0.0.1:55556";
		MClient client(std::move(clientConfig));

		for (auto i = 0ul; i < numberOfClients; ++i) {
			threads.emplace_back(std::thread([&client, &bytes, &shouldStart]() {
				boost::uuids::random_generator gen;
				boost::uuids::uuid id = gen();
				auto clientId = boost::uuids::to_string(id);
				std::string str = "Hello";
				str.resize(1024, 'a');
				auto cycles = 10'000ul;
				auto msg = str + "_" + clientId;
				while (!shouldStart) {
					std::this_thread::yield();
				}
				auto poolSize = 100;

				for (auto j = 0ul; j < cycles / poolSize; ++j) {
					std::vector<std::pair<std::future<void>, Message>> pool;
					for (auto k = 0ul; k < poolSize; ++k) {
						bytes += msg.size();
						pool.emplace_back(client.sendRequest(msg));
					}
					for (auto& message:pool) {
						if (message.second.valid()) {
							try {
								message.first.get();
								message.second.get();
							}
							catch (std::exception& ex) {
								std::cout << "Future exception: " << ex.what() << std::endl;
							}
						}
						else {
							std::cout << "Invalid future." << std::endl;
						}
					}
				}
			}));
		}
		shouldStart = true;
		auto start = std::chrono::high_resolution_clock::now();
		for (auto& thread:threads) {
			thread.join();
		}
		auto end = std::chrono::high_resolution_clock::now();
		auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		std::cout << "Througput of " << numberOfClients << " clients: " << std::fixed << double(bytes) / us * 1'000'000 / 1024 / 1024
				  << "MiB/s." << std::endl;
	}*/
	return 0;
}