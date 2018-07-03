//#define NONIUS_RUNNER
//
//#include <nonius.h++>

#include <future>
#include <functional>
#include <boost/lockfree/queue.hpp>
#include <iostream>
#include <random>
#include <fstream>
#include <vector>
#include <shared_mutex>

#include "spdk/nvme.h"
#include "NVMeManager.h"
#include "NVMeStriper.h"
#include "Throughput.h"

//std::atomic_bool go{false};
//
//struct NVMeNamespace {
//    spdk_nvme_ns* ns;
//    spdk_nvme_qpair* queue;
//};
//
//struct NVMeController {
//    spdk_nvme_ctrlr* ctrlr;
//    std::vector<NVMeNamespace> namespaces;
//};
//
//enum class NVMeVerb : uint8_t {
//    Read, Write, Flush, VerbMAX
//};
//
//struct NVMeTask {
//    NVMeTask() = delete;
//
//    explicit NVMeTask(std::promise<void>&& prom) : promise(std::move(prom)) {}
//
//    NVMeVerb nvmeVerb = NVMeVerb::VerbMAX;
//    std::promise<void> promise;
//    uint64_t addr = 0;
//    void* buffer = nullptr;
//    uint32_t bufferSize = 0;
//    decltype(std::chrono::high_resolution_clock::now()) now;
//    size_t controller = 0;
//    size_t ns = 0;
//};
//
//class NVMeIOWorker {
//public:
//    using Controllers = std::vector<NVMeController>;
//    using LocklessQueue = boost::lockfree::queue<NVMeTask*, boost::lockfree::capacity<0xFFFF - 1>>;
//
//    NVMeIOWorker() {
//        spdk_env_opts opts;
//        spdk_env_opts_init(&opts);
//        opts.name = "KuddelNVMe";
//        opts.core_mask = "0xFF";
//        opts.shm_id = 0;
//        if (spdk_env_init(&opts) < 0) {
//            throw std::runtime_error("Unable to initialize SPDK env");
//        }
//
//        auto rc = spdk_nvme_probe(NULL, this,
//                                  [](void*, const spdk_nvme_transport_id*, spdk_nvme_ctrlr_opts* controllerOptions) {
//                                      controllerOptions->io_queue_requests = std::numeric_limits<uint16_t>::max();
//                                      controllerOptions->io_queue_size = std::numeric_limits<uint16_t>::max();
//                                      return true;
//                                  }, &NVMeIOWorker::AttachClbk, NULL);
//
//        if (rc != 0) {
//            throw std::runtime_error("Failed to probe NVMe.");
//        }
//
//        if (controllers.empty())
//            throw std::runtime_error("Cant enumerate PCIe controllers.");
//
//        std::for_each(controllers.begin(), controllers.end(), [](auto& controller) {
//            std::for_each(controller.namespaces.begin(), controller.namespaces.end(), [&controller](auto& ns) {
//                spdk_nvme_io_qpair_opts qopts;
//
//                spdk_nvme_ctrlr_get_default_io_qpair_opts(controller.ctrlr, &qopts, sizeof(qopts));
//
//                auto qPair = spdk_nvme_ctrlr_alloc_io_qpair(controller.ctrlr, &qopts, sizeof(qopts));
//                if (qPair == NULL) {
//                    throw std::runtime_error("ERROR: spdk_nvme_ctrlr_alloc_io_qpair() failed.");
//                }
//                ns.queue = qPair;
//            });
//        });
//        auto buckets = controllers.size() / devicesPerThread;
//        taskQueues = std::vector<LocklessQueue>(buckets);
//        for (size_t idx = 0; idx < buckets; ++idx) {
//            eventLoops.emplace_back([this, idx]() {
//                auto thread = std::thread([this, idx]() {
//                    RunEventLoop(idx);
//                });
//                //std::cout << "Thread " << thread.get_id() << ", index " << idx << std::endl;
//                cpu_set_t cpuset;
//                CPU_ZERO(&cpuset);
//                CPU_SET(idx, &cpuset);
//                pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &cpuset);
//                return thread;
//            }());
//        }
//    }
//
//    ~NVMeIOWorker() {
//        shouldStop = true;
//        std::for_each(eventLoops.begin(), eventLoops.end(), [](auto& thread) { thread.join(); });
//        std::for_each(controllers.begin(), controllers.end(),
//                      [](const auto& controller) { spdk_nvme_detach(controller.ctrlr); });
//    }
//
//    std::future<void> Write(Controllers::size_type controller, Controllers::size_type ns, uint64_t addr, void* buffer,
//                            uint32_t bufferSize) {
//        return DoOp<static_cast<std::underlying_type_t<NVMeVerb>>(NVMeVerb::Write)>(controller, ns, addr, buffer,
//                                                                                    bufferSize);
//    }
//
//    std::future<void> Read(Controllers::size_type controller, Controllers::size_type ns, uint64_t addr, void* buffer,
//                           uint32_t bufferSize) {
//        return DoOp<static_cast<std::underlying_type_t<NVMeVerb>>(NVMeVerb::Read)>(controller, ns, addr, buffer,
//                                                                                   bufferSize);
//    }
//
//    uint64_t GetSize() const { return spdk_nvme_ns_get_size(controllers[0].namespaces[0].ns); }
//
//    uint64_t GetBlockSize() const { return spdk_nvme_ns_get_sector_size(controllers[0].namespaces[0].ns); }
//
//    Controllers::size_type ControllersCount() const {
//        return controllers.size();
//    }
//
//    size_t NamespacesCount(Controllers::size_type idx) const {
//        return controllers[idx].namespaces.size();
//    }
//
//private:
//    void RunEventLoop(size_t idx) {
//        while (!shouldStop || !taskQueues[idx].empty()) {
//            taskQueues[idx].consume_all([this](NVMeTask* task) { performTask(task); });
//            for (size_t i = 0; i < controllers.size(); ++i) {
//                if (i % (controllers.size() / devicesPerThread) == idx) {
//                    std::for_each(controllers[i].namespaces.begin(), controllers[i].namespaces.end(),
//                                  [](const auto& ns) {
//                                      spdk_nvme_qpair_process_completions(ns.queue, 0);
//                                  });
//                }
//            }
//        }
//    }
//
//    bool submitTask(NVMeTask* task) {
//        return taskQueues[task->controller % taskQueues.size()].bounded_push(task);
//    }
//
//    int performTask(NVMeTask* task) {
//        int res = 0;
//        switch (task->nvmeVerb) {
//            case NVMeVerb::Read:
//                res = spdk_nvme_ns_cmd_read(controllers[task->controller].namespaces[task->ns].ns,
//                                            controllers[task->controller].namespaces[task->ns].queue, task->buffer,
//                                            task->addr, /* LBA start */
//                                            task->bufferSize / spdk_nvme_ns_get_sector_size(
//                                                    controllers[task->controller].namespaces[task->ns].ns), /* number of LBAs */
//                                            NVMeIOWorker::CompletionClbk, task, 0);
//                break;
//            case NVMeVerb::Write:
//                res = spdk_nvme_ns_cmd_write(controllers[task->controller].namespaces[task->ns].ns,
//                                             controllers[task->controller].namespaces[task->ns].queue, task->buffer,
//                                             task->addr, /* LBA start */
//                                             task->bufferSize / spdk_nvme_ns_get_sector_size(
//                                                     controllers[task->controller].namespaces[task->ns].ns), /* number of LBAs */
//                                             NVMeIOWorker::CompletionClbk, task, 0);
//                break;
//            default:
//                if (task) {
//                    task->promise.set_exception(std::make_exception_ptr(std::runtime_error("Unknown operation.")));
//                    delete task;
//                }
//                break;
//        }
//        task->now = std::chrono::high_resolution_clock::now();
//        spdk_nvme_qpair_process_completions(controllers[task->controller].namespaces[task->ns].queue, 0);
//        if (res != 0) {
//            submitTask(task);
//        }
//        return res;
//    }
//
//    template<uint8_t Op>
//    std::future<void>
//    DoOp(Controllers::size_type controller, Controllers::size_type ns, size_t addr, void* buffer, size_t bufferSize) {
//        std::promise<void> promise;
//        auto future = promise.get_future();
//        NVMeTask* task = new NVMeTask(std::move(promise));
//        task->nvmeVerb = static_cast<NVMeVerb>(Op);
//        task->bufferSize = bufferSize;
//        task->buffer = buffer;
//        task->addr = addr;
//        task->controller = controller;
//        task->ns = ns;
//
//        size_t retries = 0;
//        while (!submitTask(task)) {
//            if (retries == 1000) {
//                std::cout << "submission failure." << std::endl;
//                promise.set_exception(std::make_exception_ptr(std::runtime_error("Failed to submit a task")));
//                delete task;
//            }
//            std::this_thread::yield();
//            ++retries;
//        }
//        return future;
//    }
//
//    static void CompletionClbk(void* arg, const spdk_nvme_cpl* completion) {
//        auto task = static_cast<NVMeTask*>(arg);
//        if (spdk_nvme_cpl_is_error(completion)) {
//            task->promise.set_exception(std::make_exception_ptr(std::runtime_error(
//                    std::string("Operation failed. Code type: ") + std::to_string(completion->status.sct) + ". Code: " +
//                    std::to_string(completion->status.sc))));
//        } else {
//            task->promise.set_value();
//        }
//        delete task;
//        //std::cout << "clbk" << std::endl;
//    }
//
//    static void AttachClbk(void* context, const spdk_nvme_transport_id* transportId, spdk_nvme_ctrlr* ctrlr,
//                           const spdk_nvme_ctrlr_opts* opts) {
//        NVMeController ctrlEntry;
//        ctrlEntry.ctrlr = ctrlr;
//
//        auto namespaceCount = spdk_nvme_ctrlr_get_num_ns(ctrlr);
//        for (unsigned namespaceId = 1; namespaceId <= namespaceCount; ++namespaceId) {
//            auto ns = spdk_nvme_ctrlr_get_ns(ctrlr, namespaceId);
//            if (ns != NULL && spdk_nvme_ns_is_active(ns)) {
//                ctrlEntry.namespaces.emplace_back(NVMeNamespace{ns});
//            }
//        }
//        static_cast<NVMeIOWorker*>(context)->controllers.emplace_back(std::move(ctrlEntry));
//    }
//
//private:
//    std::vector<LocklessQueue> taskQueues;
//    size_t devicesPerThread = 4;
//    std::atomic_bool shouldStop{false};
//    std::vector<std::thread> eventLoops;
//    Controllers controllers;
//};

constexpr size_t writerThreadsCount = 100;
constexpr size_t writerBuffSize = 4096;
constexpr size_t readerThreadsCount = 64;
constexpr size_t readerBuffSize = 4096;
constexpr size_t ioOperations = 1;
constexpr size_t iterations = 10'000;
//constexpr size_t devices2Use = 8;
//constexpr size_t devXns = devices2Use * 1;


//void WriteStress(NVMeIOWorker& worker)
//{
//	go = false;
//	std::vector<std::thread> writerThreads(writerThreadsCount);
//	generate(writerThreads.begin(), writerThreads.end(), [&worker]()
//	{
//		return std::thread([&worker]()
//						   {
//							   std::random_device rd;
//							   std::mt19937 gen(rd());
//							   std::uniform_int_distribution<uint64_t> dis(0, worker.GetSize() / worker.GetBlockSize());
//							   std::uniform_int_distribution<uint8_t> dataDistr(0, 255);
//
//							   uint8_t* writeBuff = static_cast<uint8_t*>(spdk_dma_malloc(writerBuffSize, 4096, nullptr));
//
//							   std::generate(writeBuff, writeBuff + writerBuffSize, [&dataDistr, &gen]()
//							   { return dataDistr(gen); });
//
//							   while (!go) { std::this_thread::yield(); }
//
//							   for (size_t i = 0; i < ioOperations; ++i) {
//								   try {
//									   auto future = worker.Write(dis(gen), writeBuff, writerBuffSize);
//									   if (!future.valid())
//										   throw std::runtime_error("Future is invalid!");
//									   future.get();
//								   }
//								   catch (std::exception& ex) {
//									   std::cout << "Error: " << ex.what() << std::endl;
//								   }
//							   }
//							   spdk_dma_free(writeBuff);
//						   });
//	});
//
//	go = true;
//	auto start = std::chrono::high_resolution_clock::now();
//
//	for (auto& thread:writerThreads) { thread.join(); };
//	auto end = std::chrono::high_resolution_clock::now();
//	std::cout << "Completion of " << ioOperations * writerThreads.size() << " write operations took "
//			  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms." << std::endl
//			  << (ioOperations * writerThreads.size()) / std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 1000
//			  << "IOPS" << std::endl << "Latency: "
//			  << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / (ioOperations * writerThreads.size()) << "ns."
//			  << std::endl;
//}

std::atomic_uint64_t completedCounter = 0;
std::atomic_uint64_t submitedCounter = 0;

//void ReadStress(NVMeIOWorker& worker) {
//    go = false;
//    std::vector<std::thread> readerThreads(readerThreadsCount);
//    generate(readerThreads.begin(), readerThreads.end(), [&worker]() {
//        auto thread = std::thread([&worker]() {
//            std::random_device rd;
//            std::mt19937 gen(rd());
//            std::uniform_int_distribution<uint64_t> dis(0, (worker.GetSize() / worker.GetBlockSize()) - 1 -
//                                                           (readerBuffSize / 512));
//            uint8_t* readBuff = static_cast<uint8_t*>(spdk_dma_malloc(readerBuffSize, 4096, nullptr));
//            std::vector<std::future<void>> futures;
//
//            std::uniform_int_distribution<uint64_t> ops(8, 256);
//            std::uniform_int_distribution<uint64_t> sleep(2, 20);
//            std::this_thread::sleep_for(std::chrono::seconds(sleep(gen)));
//
//            for (size_t j = 0; j < iterations; ++j) {
//                futures.clear();
//                size_t ioOperations = ops(gen) * devices2Use;
//                for (size_t i = 0; i < ioOperations; ++i) {
//                    futures.emplace_back(
//                            worker.Read(submitedCounter % devices2Use, 0, dis(gen), readBuff, readerBuffSize));
//                    ++submitedCounter;
//                }
//                std::for_each(futures.begin(), futures.end(), [](auto& future) {
//                    future.get();
//                    ++completedCounter;
//                });
//            }
////            auto sum = std::accumulate(timing.begin(), timing.end(), 0);
////            std::cout << "Average time from io submition to callback execution: " << sum / timing.size() << "us."
////                      << std::endl;
//            spdk_dma_free(readBuff);
//        });
//        cpu_set_t cpuset;
//        CPU_ZERO(&cpuset);
//        for (auto c = 4; c < 36; ++c)
//            CPU_SET(c, &cpuset);
//        pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &cpuset);
//        return thread;
//    });
////    generate(readerThreads.begin(), readerThreads.end(), [&worker]() {
////        auto thread = std::thread([&worker]() {
////            std::random_device rd;
////            std::mt19937 gen(rd());
////            std::uniform_int_distribution<uint64_t> dis(0, (worker.GetSize() / worker.GetBlockSize()) - 1 -
////                                                           (readerBuffSize / 512));
////            uint8_t* readBuff = static_cast<uint8_t*>(spdk_dma_malloc(readerBuffSize, 4096, nullptr));
////            std::vector<std::future<void>> futures;
////
////            std::uniform_int_distribution<uint64_t> ops(8, 256);
////            std::uniform_int_distribution<uint64_t> sleep(2, 20);
////            std::this_thread::sleep_for(std::chrono::seconds(sleep(gen)));
////            size_t ioOperations = 4096;
////            for (size_t i = 0; i < ioOperations; ++i) {
////                futures.emplace_back(worker.Read(submitedCounter % devices2Use, 0, dis(gen), readBuff, readerBuffSize));
////            }
////            for (size_t j = 0; j < iterations; ++j) {
////
////                for (size_t i = 0; i < ioOperations; ++i) {
////                    futures[i].get();
////                    ++completedCounter;
////                    futures[i] = worker.Read(submitedCounter % devices2Use, 0, dis(gen), readBuff, readerBuffSize);
////                }
////            }
////            spdk_dma_free(readBuff);
////        });
////        cpu_set_t cpuset;
////        CPU_ZERO(&cpuset);
////        for (auto c = 4; c < 36; ++c)
////            CPU_SET(c, &cpuset);
////        pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &cpuset);
////        return thread;
////    });
////	generate(readerThreads.begin(), readerThreads.end(), [&worker]()
////	{
////		auto thread = std::thread([&worker]()
////								  {
////									  std::random_device rd;
////									  std::mt19937 gen(rd());
////									  std::uniform_int_distribution<uint64_t> dis(0, (worker.GetSize() / worker.GetBlockSize()) - 1 -
////																					 (readerBuffSize / 512));
////									  uint8_t* readBuff = static_cast<uint8_t*>(spdk_dma_malloc(readerBuffSize, 4096, nullptr));
////									  while (true) {
////										  worker.Read(completedCounter % devices2Use, 0, dis(gen), readBuff, readerBuffSize).get();
////										  ++completedCounter;
////									  }
////									  spdk_dma_free(readBuff);
////								  });
////		cpu_set_t cpuset;
////		CPU_ZERO(&cpuset);
////		for (auto c = 1; c < 36; ++c)
////			CPU_SET(c, &cpuset);
////		pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &cpuset);
////		return thread;
////	});
//
//    auto c = std::thread([&worker]() {
//        auto start = std::chrono::high_resolution_clock::now();
//        while (true) {
//            std::this_thread::sleep_for(std::chrono::seconds(3));
//            auto end = std::chrono::high_resolution_clock::now();
//            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
//            if (completedCounter > 0 && elapsed > 0) {
//                std::cout << "Completed tasks " << completedCounter << ". Tasks in flight: "
//                          << submitedCounter - completedCounter << std::endl << completedCounter / elapsed * 1000
//                          << " IOPS" << std::endl;
//                completedCounter = 0;
//                submitedCounter = 0;
//                start = std::chrono::high_resolution_clock::now();
//            }
//        }
//    });
//    cpu_set_t cpuset;
//    CPU_ZERO(&cpuset);
//    for (auto c = 4; c < 36; ++c)
//        CPU_SET(c, &cpuset);
//    pthread_setaffinity_np(c.native_handle(), sizeof(cpu_set_t), &cpuset);
//    readerThreads.emplace_back(std::move(c));
////	go = true;
////	auto start = std::chrono::high_resolution_clock::now();
//
//    for (auto& thread:readerThreads) {
//        thread.join();
//    };
////	auto end = std::chrono::high_resolution_clock::now();
////	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
////	std::cout << "Completion of " << ioOperations * readerThreads.size() * iterations * devXns << " read operations took "
////			  << elapsed << "ms." << std::endl
////			  << (ioOperations * readerThreads.size() * iterations * devXns) / elapsed * 1000 << "IOPS"
////			  << std::endl << "Latency: "
////			  << double(elapsed * 1000) / (ioOperations * readerThreads.size() * iterations * devXns) << "us."
////			  << std::endl;
//}

void ReadStress(NVMeManager& worker)
{
    std::vector<std::thread> readerThreads(readerThreadsCount);
    generate(readerThreads.begin(), readerThreads.end(), [&worker, counter = 0]() mutable {
        auto thread = std::thread([&worker, counter]() {
            auto striper = NVMeStriper(worker.GetControllers()[counter % worker.GetControllers().size()], worker);
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<uint64_t> dis(0, striper.GetSize() / striper.GetBlockSize() -
                                                           readerBuffSize / striper.GetBlockSize());
            uint8_t* readBuff = static_cast<uint8_t*>(spdk_dma_malloc(readerBuffSize, 4096, nullptr));
            std::vector<std::future<void>> futures;

            std::uniform_int_distribution<uint64_t> ops(64, 2048);

            for (size_t j = 0; j < iterations; ++j) {
                futures.clear();
                size_t ioOperations = ops(gen);// * devices2Use;
                for (size_t i = 0; i < ioOperations; ++i) {
                    futures.emplace_back(striper.Read(dis(gen), readBuff, readerBuffSize));
                    ++submitedCounter;
                }
                std::for_each(futures.begin(), futures.end(), [](auto& future) {
                    future.get();
                    ++completedCounter;
                });
            }
            spdk_dma_free(readBuff);
        });
        ++counter;
//        cpu_set_t cpuset;
//        CPU_ZERO(&cpuset);
//        for (auto c = 4; c < 36; ++c)
//            CPU_SET(c, &cpuset);
//        pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &cpuset);
        return thread;
    });

    auto c = std::thread([&worker]() {
        auto start = std::chrono::high_resolution_clock::now();
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            if (completedCounter > 0 && elapsed > 0) {
                std::cout << "Completed tasks " << completedCounter << ". Tasks in flight: "
                          << submitedCounter - completedCounter << std::endl << completedCounter / elapsed * 1000
                          << " IOPS" << std::endl;
                completedCounter = 0;
                submitedCounter = 0;
                start = std::chrono::high_resolution_clock::now();
            }
        }
    });
//    cpu_set_t cpuset;
//    CPU_ZERO(&cpuset);
//    for (auto c = 4; c < 36; ++c)
//        CPU_SET(c, &cpuset);
//    pthread_setaffinity_np(c.native_handle(), sizeof(cpu_set_t), &cpuset);
    readerThreads.emplace_back(std::move(c));

    for (auto& thread:readerThreads) {
        thread.join();
    };
}

void ValidityTest(NVMeManager& worker, size_t counter)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    auto striper = NVMeStriper(worker.GetControllers()[counter % worker.GetControllers().size()], worker);
    std::uniform_int_distribution<uint64_t> dis(0, striper.GetSize());
    std::uniform_int_distribution<uint8_t> dataDistr(0, 255);

    uint8_t* writeBuff = static_cast<uint8_t*>(spdk_dma_malloc(writerBuffSize, 4096, nullptr));

    bool testPassed = true;
    for (size_t i = 0; i < ioOperations; ++i) {
        try {
            std::generate(writeBuff, writeBuff + writerBuffSize, [&dataDistr, &gen]() { return dataDistr(gen); });
            std::vector<uint8_t> corpus(writeBuff, writeBuff + writerBuffSize);
            auto addr = dis(gen);

            {
                auto future = striper.Write(addr, writeBuff, writerBuffSize);
                if (!future.valid()) {
                    throw std::runtime_error("Future is invalid!");
                }
                future.get();
            }
            std::fill(writeBuff, writeBuff + writerBuffSize, 0);
            {
                auto future = striper.Read(addr, writeBuff, writerBuffSize);
                if (!future.valid()) {
                    throw std::runtime_error("Future is invalid!");
                }
                future.get();
                if (memcmp(writeBuff, corpus.data(), writerBuffSize) != 0) {
                    std::cout << "Buffers does not match" << std::endl;
                    testPassed = false;
                }
            }
        } catch (std::exception& ex) {
            std::cout << "Error: " << ex.what() << std::endl;
        }
    }
    spdk_dma_free(writeBuff);

    if (testPassed) {
        std::cout << "Validity test passed successfully." << std::endl;
    } else {
        std::cout << "Validity test failed." << std::endl;
    }
}

void ErrorHandlingTest(NVMeManager& worker, size_t counter)
{
    uint8_t* buff = static_cast<uint8_t*>(spdk_dma_malloc(4096, 4096, nullptr));
    auto striper = NVMeStriper(worker.GetControllers()[counter % worker.GetControllers().size()], worker);
    std::cout << "Write failures." << std::endl;
    {
        try {
            std::cout << "write(std::numeric_limits<size_t>::max(), buff, writerBuffSize);" << std::endl;
            auto future = striper.Write(std::numeric_limits<size_t>::max(), buff, writerBuffSize);
            if (!future.valid()) {
                throw std::runtime_error("Future is invalid!");
            }
            future.get();
        } catch (std::exception& ex) {
            std::cout << "Failed to perform operation. Reason: " << ex.what() << std::endl;
        }
    }
    {
        try {
            std::cout << "write(std::numeric_limits<size_t>::min(), buff, writerBuffSize);" << std::endl;
            auto future = striper.Write(std::numeric_limits<size_t>::min(), buff, writerBuffSize);
            if (!future.valid()) {
                throw std::runtime_error("Future is invalid!");
            }
            future.get();

        } catch (std::exception& ex) {
            std::cout << "Failed to perform operation. Reason: " << ex.what() << std::endl;
        }
    }
    {
        try {
            std::cout << "write(size_t(0), nullptr, writerBuffSize);" << std::endl;
            auto future = striper.Write(size_t(0), nullptr, writerBuffSize);
            if (!future.valid()) {
                throw std::runtime_error("Future is invalid!");
            }
            future.get();
        } catch (std::exception& ex) {
            std::cout << "Failed to perform operation. Reason: " << ex.what() << std::endl;
        }
    }
    {
        try {
            std::cout << "write(size_t(0), buff, writerBuffSize * 2);" << std::endl;
            auto future = striper.Write(size_t(0), buff, writerBuffSize * 2);
            if (!future.valid()) {
                throw std::runtime_error("Future is invalid!");
            }
            future.get();
        } catch (std::exception& ex) {
            std::cout << "Failed to perform operation. Reason: " << ex.what() << std::endl;
        }
    }
    {
        try {
            std::vector<std::byte> invaliBuff(1234, std::byte(0));
            std::cout << "write(size_t(0), invaliBuff.data(), invaliBuff.size());" << std::endl;
            auto future = striper.Write(size_t(0), reinterpret_cast<const byte*>(invaliBuff.data()), invaliBuff.size());
            if (!future.valid()) {
                throw std::runtime_error("Future is invalid!");
            }
            future.get();
        } catch (std::exception& ex) {
            std::cout << "Failed to perform operation. Reason: " << ex.what() << std::endl;
        }
    }

    {
        try {
            std::vector<std::byte> invaliBuff(4096, std::byte(0));
            std::cout << "write(size_t(0), invaliBuff.data(), invaliBuff.size());" << std::endl;
            auto future = striper.Write(size_t(0), reinterpret_cast<const byte*>(invaliBuff.data()), invaliBuff.size());
            if (!future.valid()) {
                throw std::runtime_error("Future is invalid!");
            }
            future.get();
        } catch (std::exception& ex) {
            std::cout << "Failed to perform operation. Reason: " << ex.what() << std::endl;
        }
    }

    std::cout << "Read failures." << std::endl;
    {
        try {
            std::cout << "read(std::numeric_limits<size_t>::max(), buff, readerBuffSize);" << std::endl;
            auto future = striper.Read(std::numeric_limits<size_t>::max(), buff, readerBuffSize);
            if (!future.valid()) {
                throw std::runtime_error("Future is invalid!");
            }
            future.get();
        } catch (std::exception& ex) {
            std::cout << "Failed to perform operation. Reason: " << ex.what() << std::endl;
        }
    }
    {
        try {
            std::cout << "read(std::numeric_limits<size_t>::min(), buff, readerBuffSize);" << std::endl;
            auto future = striper.Read(std::numeric_limits<size_t>::min(), buff, readerBuffSize);
            if (!future.valid()) {
                throw std::runtime_error("Future is invalid!");
            }
            future.get();
        } catch (std::exception& ex) {
            std::cout << "Failed to perform operation. Reason: " << ex.what() << std::endl;
        }
    }
    {
        try {
            std::cout << "read(size_t(0), nullptr, readerBuffSize);" << std::endl;
            auto future = striper.Read(size_t(0), nullptr, readerBuffSize);
            if (!future.valid()) {
                throw std::runtime_error("Future is invalid!");
            }
            future.get();
        } catch (std::exception& ex) {
            std::cout << "Failed to perform operation. Reason: " << ex.what() << std::endl;
        }
    }
    {
        try {
            std::cout << "read(size_t(0), buff, readerBuffSize * 2);" << std::endl;
            auto future = striper.Read(size_t(0), buff, readerBuffSize * 2);
            if (!future.valid()) {
                throw std::runtime_error("Future is invalid!");
            }
            future.get();
        } catch (std::exception& ex) {
            std::cout << "Failed to perform operation. Reason: " << ex.what() << std::endl;
        }
    }
    {
        try {
            std::cout << "read(size_t(0), invaliBuff.data(), invaliBuff.size());" << std::endl;
            std::vector<std::byte> invaliBuff(1234, std::byte(0));
            auto future = striper.Read(size_t(0), reinterpret_cast<byte*>(invaliBuff.data()), invaliBuff.size());
            if (!future.valid()) {
                throw std::runtime_error("Future is invalid!");
            }
            future.get();
        } catch (std::exception& ex) {
            std::cout << "Failed to perform operation. Reason: " << ex.what() << std::endl;
        }
    }
    {
        try {
            std::cout << "read(size_t(0), invaliBuff.data(), invaliBuff.size());" << std::endl;
            std::vector<std::byte> invaliBuff(4096, std::byte(0));
            auto future = striper.Read(size_t(0), reinterpret_cast<byte*>(invaliBuff.data()), invaliBuff.size());
            if (!future.valid()) {
                throw std::runtime_error("Future is invalid!");
            }
            future.get();
        } catch (std::exception& ex) {
            std::cout << "Failed to perform operation. Reason: " << ex.what() << std::endl;
        }
    }

    {
        try {
            std::cout << "read(size_t(0), invaliBuff.data(), invaliBuff.size());" << std::endl;
            constexpr size_t memSize = 2 * 1014 * 1024;
            auto hpPointer = mmap(NULL, memSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1,
                                  0);
            spdk_mem_register(hpPointer, memSize);
            auto future = striper.Read(size_t(0), reinterpret_cast<byte*>(hpPointer), memSize);
            if (!future.valid()) {
                throw std::runtime_error("Future is invalid!");
            }
            future.get();
            spdk_mem_unregister(hpPointer, memSize);
        } catch (std::exception& ex) {
            std::cout << "Failed to perform operation. Reason: " << ex.what() << std::endl;
        }
    }

    spdk_dma_free(buff);
}

//std::unique_ptr<NVMeManager> worker = nullptr;
//NONIUS_BENCHMARK("NVMe Read", [](nonius::chronometer meter) {
//    if (!worker)
//        worker = std::make_unique<NVMeManager>();
//    std::random_device rd;
//    std::mt19937 gen(rd());
//    std::uniform_int_distribution<uint64_t> dis(0, (worker->GetSize() / worker->GetBlockSize()) - 1);
//    uint8_t* readBuff = static_cast<uint8_t*>(spdk_dma_malloc(readerBuffSize, 4096, nullptr));
//    meter.measure([readBuff, &dis, &gen](int i) {
//        return worker->Read(i % 8, 0, dis(gen), readBuff, readerBuffSize).get();
//
//    });
//})

std::unique_ptr<NVMeManager> nvmeManager;

void foo()
{
    auto completion = std::async(std::launch::async, []() { nvmeManager = std::make_unique<NVMeManager>(); });
    completion.get();
}

void work()
{
    auto then = std::chrono::high_resolution_clock::now();
    while(true)
    {
        getuid();
        auto now = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::microseconds>(now-then).count()>100)
            break;
    }
}
int main(int argc, char** argv)
{
    //foo();
    //NVMeManager nvmeManager;
//    std::copy(nvmeManager->GetControllers().begin(), nvmeManager->GetControllers().end(),
//              std::ostream_iterator<std::string>(std::cout, "\n"));

//    std::for_each(nvmeManager->GetControllers().begin(), nvmeManager->GetControllers().end(),
//                  [](const auto& ctrl) {
//                      std::cout << "Zeroing " << ctrl << std::endl;
//                      nvmeManager->WriteZ(ctrl, 0, nullptr,
//                                          nvmeManager->GetSize(ctrl) * nvmeManager->GetBlockSize(ctrl)).get();
//                  });
//    uint8_t* readBuff = static_cast<uint8_t*>(spdk_dma_malloc(4096, 4096, nullptr));
//    Throughput test([readBuff]() {
//        nvmeManager->Read(nvmeManager->GetControllers()[0], 0, readBuff, 4096);
//        return 4096;
//    }, 100000, 16, 1);
//    std::ofstream ofs("ouput.img", std::ios::binary | std::ios::out);
//    ofs.seekp((300<<20) - 1);
//    ofs.write("", 1);
    {
        Throughput test([]() {
            work();
            return 1;
        }, 2);
        test.Start();
        test.Stop();
    }
//    {
//        Throughput test([]() {
//            work();
//            return 1;
//        }, 1'000, 50, 2);
//        test.Start();
//        //std::this_thread::sleep_for(std::chrono::seconds(150));
//        test.Stop();
//    }
    //NVMeIOWorker worker;
//    ErrorHandlingTest(*nvmeManager, 0);
//    ErrorHandlingTest(*nvmeManager, 2);
//    ErrorHandlingTest(*nvmeManager, 4);
//    ErrorHandlingTest(*nvmeManager, 6);
//    ErrorHandlingTest(*nvmeManager, 7);

    //ValidityTest(*nvmeManager, 0);
//    ValidityTest(*nvmeManager, 1);
//    ValidityTest(*nvmeManager, 2);
//    ValidityTest(*nvmeManager, 3);
//    ValidityTest(*nvmeManager, 4);
//    ValidityTest(*nvmeManager, 5);
//    ValidityTest(*nvmeManager, 6);
//    ValidityTest(*nvmeManager, 7);

    //WriteStress(worker);

    //ReadStress(*nvmeManager);

//    auto striper = NVMeStriper(nvmeManager->GetControllers()[0], *nvmeManager);
//    size_t s = (1562824368 - 1562824352) * striper.GetBlockSize();
//    std::cout << "Size to read/write: " << s << std::endl;
//    uint8_t* writeBuff = static_cast<uint8_t*>(spdk_dma_zmalloc(s, 4096, nullptr));
//    auto future = striper.Write(1562824352, writeBuff, s);
//    if (!future.valid()) {
//        throw std::runtime_error("Future is invalid!");
//    }
//    future.get();
//    auto future = striper.Read(1562824352, writeBuff, s);
//    future.get();
//    std::ofstream file("formatted.bin", std::ios::binary | std::ios::out);
//    if (file.is_open()) {
//        file.write(reinterpret_cast<const char*>(writeBuff), s);
//    }
//    file.close();
//    Throughput test([counter = 0]() mutable {
//        thread_local auto striper = NVMeStriper(
//                nvmeManager->GetControllers()[counter % nvmeManager->GetControllers().size()], *nvmeManager);
//        ++counter;
//        thread_local std::random_device rd;
//        thread_local std::mt19937 gen(rd());
//        thread_local std::uniform_int_distribution<uint64_t> dis(0, striper.GetSize() -
//                                                                    readerBuffSize / striper.GetBlockSize());
//        thread_local uint8_t* readBuff = static_cast<uint8_t*>(spdk_dma_malloc(readerBuffSize, 4096, nullptr));
//        auto ret = striper.Read(dis(gen), readBuff, readerBuffSize);
//        ret.get();
//        return readerBuffSize;
//    }, 3);
//
//    THROUGHPUT_TEST([counter = 0]() mutable {
//        auto striper = NVMeStriper(nvmeManager->GetControllers()[counter % nvmeManager->GetControllers().size()],
//                                   *nvmeManager);
//        ++counter;
//        constexpr uint64_t readerBuffSize = 4096;
//        std::random_device rd;
//        std::mt19937 gen(rd());
//        std::uniform_int_distribution<uint64_t> dis(0, striper.GetSize() - readerBuffSize / striper.GetBlockSize());
//        uint8_t* readBuff = static_cast<uint8_t*>(spdk_dma_malloc(readerBuffSize, 4096, nullptr));
//        tester.add_test([&dis, &gen, readBuff, readerBuffSize]() {
//            auto ret = striper.Read(dis(gen), readBuff, readerBuffSize);
//            ret.get();
//            return readerBuffSize;
//        });
//    }, 3);
//    std::thread([]() {
//        auto striper = NVMeStriper(nvmeManager->GetControllers()[counter % nvmeManager->GetControllers().size()],
//                                   *nvmeManager);
//        ++counter;
//        constexpr uint64_t readerBuffSize = 4096;
//        std::random_device rd;
//        std::mt19937 gen(rd());
//        std::uniform_int_distribution<uint64_t> dis(0, striper.GetSize() - readerBuffSize / striper.GetBlockSize());
//        uint8_t* readBuff = static_cast<uint8_t*>(spdk_dma_malloc(readerBuffSize, 4096, nullptr));
//        while (true) {
//            myLambda();
//        }
//    });
    return 0;
}