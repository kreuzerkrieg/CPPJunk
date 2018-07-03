#pragma once

#include <cstddef>
#include <atomic>
#include <thread>
#include <vector>
#include <future>
#include <boost/lockfree/queue.hpp>
#include <map>

struct spdk_nvme_ns;
struct spdk_nvme_qpair;
struct spdk_nvme_cpl;
struct spdk_nvme_ctrlr;
struct spdk_nvme_transport_id;
struct spdk_nvme_ctrlr_opts;

struct NVMeNamespace
{
    spdk_nvme_ns* ns = nullptr;
    spdk_nvme_qpair* queue = nullptr;

    ~NVMeNamespace();
};

struct NVMeController
{
    size_t assignedBucket = 0;
    spdk_nvme_ctrlr* ctrlr = nullptr;
    std::vector<NVMeNamespace> namespaces;
};

enum class NVMeVerb : uint8_t
{
    Read, Write, WriteZ, Flush, VerbMAX
};

struct NVMeTask
{
    NVMeTask() = delete;

    explicit NVMeTask(std::promise<void>&& prom) : promise(std::move(prom))
    {}

    NVMeVerb nvmeVerb = NVMeVerb::VerbMAX;
    std::promise<void> promise;
    uint64_t addr = 0;
    void* buffer = nullptr;
    uint32_t bufferSize = 0;
    size_t bucket = 0;
    size_t controllerIdx = 0;
};

class NVMeManager
{
public:
    NVMeManager();

    ~NVMeManager();

    void AttachDevice(const std::string& deviceName);

    std::future<void> Write(const std::string& controllerName, uint64_t addr, void* buffer, uint32_t bufferSize);

    std::future<void> WriteZ(const std::string& controllerName, uint64_t addr, void* buffer, uint32_t bufferSize);

    std::future<void> Read(const std::string& controllerName, uint64_t addr, void* buffer, uint32_t bufferSize);

    uint64_t GetSize(const std::string& deviceName) const;

    unsigned int GetBlockSize(const std::string& deviceName) const;

    const std::vector<std::string>& GetControllers() const;

private:
    using Controllers = std::map<std::string, NVMeController>;
    using LocklessQueue = boost::lockfree::queue<NVMeTask*, boost::lockfree::capacity<0xFFFF - 1>>;

    void RunEventLoop(size_t idx);

    bool submitTask(NVMeTask* task);

    int performTask(NVMeTask* task);

    template<uint8_t Op>
    std::future<void> DoOp(const std::string& controllerName, size_t addr, void* buffer, size_t bufferSize);

    static void CompletionClbk(void* arg, const spdk_nvme_cpl* completion);

    static void AttachClbk(void* context, const spdk_nvme_transport_id* transportId, spdk_nvme_ctrlr* ctrlr,
                           const spdk_nvme_ctrlr_opts* opts);

private:
    std::vector<LocklessQueue> taskQueues;
    size_t devicesPerThread = 4;
    std::atomic_bool shouldStop{false};
    std::vector<std::thread> eventLoops;
    Controllers controllers;
    std::vector<std::string> controllerIds;
};
