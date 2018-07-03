//#include <spdk/nvme
#include <spdk/nvme.h>
#include "NVMeManager.h"

NVMeManager::NVMeManager()
{
    spdk_env_opts opts;
    spdk_env_opts_init(&opts);
    opts.name = "KuddelNVMe";
//    opts.core_mask = "0xFFFF";
//    opts.master_core = 1;
    opts.shm_id = 0;
    if (spdk_env_init(&opts) < 0) {
        throw std::runtime_error("Unable to initialize SPDK env");
    }

    auto rc = spdk_nvme_probe(NULL, this,
                              [](void*, const spdk_nvme_transport_id*, spdk_nvme_ctrlr_opts* controllerOptions) {
                                  controllerOptions->io_queue_requests = std::numeric_limits<uint16_t>::max();
                                  controllerOptions->io_queue_size = std::numeric_limits<uint16_t>::max();
                                  return true;
                              }, &NVMeManager::AttachClbk, NULL);

    if (rc != 0) {
        throw std::runtime_error("Failed to probe NVMe.");
    }

    if (controllers.empty()) {
        throw std::runtime_error("Cant enumerate PCIe controllers.");
    }

    size_t controllerIdx = 0;
    std::for_each(controllers.begin(), controllers.end(), [this, &controllerIdx](auto& controller) {
        controller.second.assignedBucket = controllerIdx / devicesPerThread;
        std::for_each(controller.second.namespaces.begin(), controller.second.namespaces.end(),
                      [&controllerIdx, &controller](auto& ns) {
                          spdk_nvme_io_qpair_opts qopts;

                          spdk_nvme_ctrlr_get_default_io_qpair_opts(controller.second.ctrlr, &qopts, sizeof(qopts));

                          auto qPair = spdk_nvme_ctrlr_alloc_io_qpair(controller.second.ctrlr, &qopts, sizeof(qopts));
                          if (qPair == NULL) {
                              throw std::runtime_error("ERROR: spdk_nvme_ctrlr_alloc_io_qpair() failed.");
                          }
                          ns.queue = qPair;
                      });
        ++controllerIdx;
    });
    auto buckets = controllers.size() / devicesPerThread;
    taskQueues = std::vector<LocklessQueue>(buckets);
    for (size_t idx = 0; idx < buckets; ++idx) {
        eventLoops.emplace_back([this, idx]() {
            auto thread = std::thread([this, idx]() {
                RunEventLoop(idx);
            });
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(idx + 1, &cpuset);
            pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &cpuset);
            return thread;
        }());
    }
}

std::future<void> NVMeManager::Read(const std::string& controllerName, uint64_t addr, void* buffer, uint32_t bufferSize)
{
    return DoOp<static_cast<std::underlying_type_t<NVMeVerb>>(NVMeVerb::Read)>(controllerName, addr, buffer,
                                                                               bufferSize);
}

std::future<void>
NVMeManager::Write(const std::string& controllerName, uint64_t addr, void* buffer, uint32_t bufferSize)
{
    return DoOp<static_cast<std::underlying_type_t<NVMeVerb>>(NVMeVerb::Write)>(controllerName, addr, buffer,
                                                                                bufferSize);
}

std::future<void>
NVMeManager::WriteZ(const std::string& controllerName, uint64_t addr, void* buffer, uint32_t bufferSize)
{
    return DoOp<static_cast<std::underlying_type_t<NVMeVerb>>(NVMeVerb::WriteZ)>(controllerName, addr, buffer,
                                                                                bufferSize);
}
void NVMeManager::RunEventLoop(size_t idx)
{
    while (!shouldStop || !taskQueues[idx].empty()) {
        taskQueues[idx].consume_all([this](NVMeTask* task) { performTask(task); });
        std::for_each(controllers.begin(), controllers.end(), [idx](const auto& controller) {
            if (controller.second.assignedBucket == idx) {
                std::for_each(controller.second.namespaces.begin(), controller.second.namespaces.end(),
                              [](const auto& ns) {
                                  spdk_nvme_qpair_process_completions(ns.queue, 0);
                              });
            }
        });
    }
}

bool NVMeManager::submitTask(NVMeTask* task)
{
    return taskQueues[task->bucket].bounded_push(task);
}

int NVMeManager::performTask(NVMeTask* task)
{
    int res = 0;
    auto beg = controllers.begin();
    std::advance(beg, task->controllerIdx);
    switch (task->nvmeVerb) {
        case NVMeVerb::Read:
            res = spdk_nvme_ns_cmd_read(beg->second.namespaces[0].ns, beg->second.namespaces[0].queue, task->buffer,
                                        task->addr,
                                        task->bufferSize / spdk_nvme_ns_get_sector_size(beg->second.namespaces[0].ns),
                                        NVMeManager::CompletionClbk, task, 0);
            break;
        case NVMeVerb::Write:
            res = spdk_nvme_ns_cmd_write(beg->second.namespaces[0].ns, beg->second.namespaces[0].queue, task->buffer,
                                         task->addr,
                                         task->bufferSize / spdk_nvme_ns_get_sector_size(beg->second.namespaces[0].ns),
                                         NVMeManager::CompletionClbk, task, 0);
            break;
        case NVMeVerb::WriteZ:
            res = spdk_nvme_ns_cmd_write_zeroes(beg->second.namespaces[0].ns, beg->second.namespaces[0].queue,
                                                task->addr, task->bufferSize /
                                                            spdk_nvme_ns_get_sector_size(beg->second.namespaces[0].ns),
                                                NVMeManager::CompletionClbk, task, 0);
            break;
        default:
            if (task) {
                task->promise.set_exception(std::make_exception_ptr(std::runtime_error("Unknown operation.")));
                delete task;
            }
            break;
    }
    spdk_nvme_qpair_process_completions(beg->second.namespaces[0].queue, 0);
    if (res != 0) {
        submitTask(task);
    }
    return res;
}

template<uint8_t Op>
std::future<void> NVMeManager::DoOp(const std::string& controllerName, size_t addr, void* buffer, size_t bufferSize)
{
    std::promise<void> promise;
    auto future = promise.get_future();
    NVMeTask* task = new NVMeTask(std::move(promise));
    task->nvmeVerb = static_cast<NVMeVerb>(Op);
    task->bufferSize = bufferSize;
    task->buffer = buffer;
    task->addr = addr;
    task->bucket = controllers[controllerName].assignedBucket;
    task->controllerIdx = std::distance(controllers.begin(), controllers.find(controllerName));

    size_t retries = 0;
    while (!submitTask(task)) {
        if (retries == 1000) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error("Failed to submit a task")));
            delete task;
        }
        std::this_thread::yield();
        ++retries;
    }
    return future;
}

void NVMeManager::CompletionClbk(void* arg, const spdk_nvme_cpl* completion)
{
    auto task = static_cast<NVMeTask*>(arg);
    if (spdk_nvme_cpl_is_error(completion)) {
        task->promise.set_exception(std::make_exception_ptr(std::runtime_error(
                std::string("Operation failed. Code type: ") + std::to_string(completion->status.sct) + ". Code: " +
                std::to_string(completion->status.sc))));
    } else {
        task->promise.set_value();
    }
    delete task;
}

void NVMeManager::AttachClbk(void* context, const spdk_nvme_transport_id* transportId, spdk_nvme_ctrlr* ctrlr,
                             const spdk_nvme_ctrlr_opts* opts)
{
    NVMeController ctrlEntry;
    ctrlEntry.ctrlr = ctrlr;

    auto namespaceCount = spdk_nvme_ctrlr_get_num_ns(ctrlr);
    for (unsigned namespaceId = 1; namespaceId <= namespaceCount; ++namespaceId) {
        auto ns = spdk_nvme_ctrlr_get_ns(ctrlr, namespaceId);
        if (ns != NULL && spdk_nvme_ns_is_active(ns)) {
            ctrlEntry.namespaces.emplace_back(NVMeNamespace{ns});
        }
    }
    std::string transportName = "trid:";
    transportName += transportId->traddr;
    static_cast<NVMeManager*>(context)->controllers[transportName] = ctrlEntry;
    static_cast<NVMeManager*>(context)->controllerIds.push_back(transportName);
}

NVMeManager::~NVMeManager()
{
    shouldStop = true;
    std::for_each(eventLoops.begin(), eventLoops.end(), [](auto& thread) { thread.join(); });
    std::for_each(controllers.begin(), controllers.end(),
                  [](const auto& controller) { spdk_nvme_detach(controller.second.ctrlr); });
}

const std::vector<std::string>& NVMeManager::GetControllers() const
{
    return controllerIds;
}

uint64_t NVMeManager::GetSize(const std::string& deviceName) const
{
    auto ns = controllers.at(deviceName).namespaces[0].ns;
    return spdk_nvme_ns_get_size(ns) / spdk_nvme_ns_get_sector_size(ns);
}

unsigned int NVMeManager::GetBlockSize(const std::string& deviceName) const
{
    auto ns = controllers.at(deviceName).namespaces[0].ns;
    return spdk_nvme_ns_get_sector_size(ns);
}


NVMeNamespace::~NVMeNamespace()
{
//    if (queue != nullptr) {
//        spdk_nvme_ctrlr_free_io_qpair(queue);
//    }
}
