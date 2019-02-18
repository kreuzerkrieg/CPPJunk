#pragma once

#include <vector>
#include <thread>
#include <future>
#include <boost/lockfree/queue.hpp>
#include <spdk/nvme.h>

class spdk_nvme_ctrlr_opts;

class NVMeManager
{
public:
	explicit NVMeManager(const std::string& pciAddress);

	~NVMeManager();

	std::future<void> Write(uint64_t addr, void* buffer, uint32_t bufferSize);

	std::future<void> Read(uint64_t addr, void* buffer, uint32_t bufferSize);

	uint64_t GetSize() const;

	unsigned int GetBlockSize() const;

private:
	struct NVMeNamespace
	{
		spdk_nvme_ns* ns = nullptr;
		spdk_nvme_qpair* queue = nullptr;
		uint64_t nsSize = 0;
		uint64_t nsBlockSize = 0;
	};

	struct NVMeController
	{
		std::vector<NVMeNamespace> namespaces;
		std::string transportName;
		spdk_nvme_ctrlr* ctrlr = nullptr;
	};

	enum class NVMeVerb : uint8_t
	{
		Read, Write, Flush, VerbMAX
	};

	struct NVMeTask
	{
		NVMeTask() = delete;

		explicit NVMeTask(std::promise<void> promParam);

		NVMeVerb nvmeVerb = NVMeVerb::VerbMAX;
		std::promise<void> prom;
		uint64_t addr = 0;
		void* buffer = nullptr;
		uint32_t bufferSize = 0;
	};

private:
	using LocklessQueue = boost::lockfree::queue<NVMeTask*, boost::lockfree::capacity<0xFFFF - 1>>;

	void Init();

	void RunEventLoop();

	bool submitTask(NVMeTask* task);

	int performTask(NVMeTask* task);

	template<uint8_t Op>
	std::future<void> DoOp(size_t addr, void* buffer, size_t bufferSize);

	static void CompletionClbk(void* arg, const spdk_nvme_cpl* completion);

	static void AttachClbk(void* context, const spdk_nvme_transport_id* transportId, spdk_nvme_ctrlr* ctrlr, const spdk_nvme_ctrlr_opts*);

private:
	LocklessQueue taskQueues;
	std::thread loopService;
	NVMeController controller;
	spdk_nvme_transport_id trid;
	bool shouldStop = false;
};
