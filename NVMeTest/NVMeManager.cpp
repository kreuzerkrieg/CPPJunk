#include "NVMeManager.h"
#include <algorithm>
#include <cstdarg>

using namespace std;

static string FormatString(const char* format, ...)
{

	va_list ap1;
	va_start(ap1, format);

	va_list ap2;
	va_copy(ap2, ap1);

	char temp;
	int ret = vsnprintf(&temp, sizeof(temp), format, ap1);

	va_end(ap1);

	vector<char> result(ret + 1);
	ret = vsnprintf(&result[0], result.size(), format, ap2);

	va_end(ap2);

	return &result[0];
}

NVMeManager::NVMeManager(const string& pciAddress)
{
	memset(&trid, 0, sizeof(trid));
	trid.trtype = SPDK_NVME_TRANSPORT_PCIE;
	snprintf(trid.subnqn, sizeof(trid.subnqn), "%s", SPDK_NVMF_DISCOVERY_NQN);

	if (spdk_nvme_transport_id_parse(&trid, pciAddress.c_str()) != 0) {
		throw runtime_error(FormatString("Invalid transport ID format '%s'", pciAddress.c_str()));
	}

	Init();
}

void NVMeManager::Init()
{
	spdk_env_opts opts;
	spdk_env_opts_init(&opts);
	opts.name = "KuddelNVMe";
//	opts.is_driver = true;
	opts.shm_id = -1;
	opts.num_pci_addr = 1;
	opts.pci_whitelist = static_cast<spdk_pci_addr*>(malloc(sizeof(opts.pci_whitelist) * opts.num_pci_addr));
	if (spdk_pci_addr_parse(opts.pci_whitelist, trid.traddr) != 0) {
		throw runtime_error(FormatString("Invalid PCI address '%s'\n", trid.traddr));
	}
	if (spdk_env_init(&opts) < 0) {
		throw runtime_error("Unable to initialize SPDK env");
	}
	printf("%s", "Finished initializing SPDK environment");

	printf("Probing %s", trid.traddr);
	auto rc = spdk_nvme_probe(nullptr, this, [](void*, const spdk_nvme_transport_id* transportId, spdk_nvme_ctrlr_opts* controllerOptions) {
		printf("Probe callback for %s called.", transportId->traddr);
		controllerOptions->io_queue_requests = ::numeric_limits<uint16_t>::max();
		controllerOptions->io_queue_size = ::numeric_limits<uint16_t>::max();
		return true;
	}, &AttachClbk, NULL);

	if (rc != 0) {
		throw runtime_error("Failed to probe NVMe.");
	}

	if (!controller.ctrlr) {
		throw runtime_error("Cant enumerate PCIe controllers.");
	}

	for_each(controller.namespaces.begin(), controller.namespaces.end(), [this](auto& ns) {
		spdk_nvme_io_qpair_opts qopts;

		spdk_nvme_ctrlr_get_default_io_qpair_opts(controller.ctrlr, &qopts, sizeof(qopts));

		auto qPair = spdk_nvme_ctrlr_alloc_io_qpair(controller.ctrlr, &qopts, sizeof(qopts));
		if (qPair == nullptr) {
			throw runtime_error("spdk_nvme_ctrlr_alloc_io_qpair() failed.");
		}
		ns.queue = qPair;
		ns.nsSize = spdk_nvme_ns_get_size(ns.ns) / spdk_nvme_ns_get_sector_size(ns.ns);
		ns.nsBlockSize = spdk_nvme_ns_get_sector_size(ns.ns);
	});
	loopService = thread([this]() { RunEventLoop(); });
}

future<void> NVMeManager::Read(uint64_t addr, void* buffer, uint32_t bufferSize)
{
	return DoOp<static_cast<underlying_type_t<NVMeVerb>>(NVMeVerb::Read)>(addr, buffer, bufferSize);
}

future<void> NVMeManager::Write(uint64_t addr, void* buffer, uint32_t bufferSize)
{
	return DoOp<static_cast<underlying_type_t<NVMeVerb>>(NVMeVerb::Write)>(addr, buffer, bufferSize);
}

void NVMeManager::RunEventLoop()
{
	while (!shouldStop || !taskQueues.empty()) {
		taskQueues.consume_all([this](NVMeTask* task) {
			performTask(task);
		});
		for_each(controller.namespaces.begin(), controller.namespaces.end(), [](const auto& ns) {
			spdk_nvme_qpair_process_completions(ns.queue, 0);
		});
	}
}

bool NVMeManager::submitTask(NVMeTask* task)
{
	auto retVal = taskQueues.bounded_push(task);
	return retVal;
}

int NVMeManager::performTask(NVMeTask* task)
{
	int res = 0;
	switch (task->nvmeVerb) {
	case NVMeVerb::Read:
		res = spdk_nvme_ns_cmd_read(controller.namespaces[0].ns, controller.namespaces[0].queue, task->buffer, task->addr,
									task->bufferSize / spdk_nvme_ns_get_sector_size(controller.namespaces[0].ns),
									NVMeManager::CompletionClbk, task, 0);
		break;
	case NVMeVerb::Write:
		res = spdk_nvme_ns_cmd_write(controller.namespaces[0].ns, controller.namespaces[0].queue, task->buffer, task->addr,
									 task->bufferSize / spdk_nvme_ns_get_sector_size(controller.namespaces[0].ns),
									 NVMeManager::CompletionClbk, task, 0);
		break;
	default:
		if (task) {
			task->prom.set_exception(make_exception_ptr(runtime_error("Unknown operation.")));
			delete task;
		}
		break;
	}
	if (res != 0) {
		submitTask(task);
	}
	return res;
}

template<uint8_t Op>
future<void> NVMeManager::DoOp(size_t addr, void* buffer, size_t bufferSize)
{
	promise<void> promise;
	auto future = promise.get_future();
	NVMeTask* task = new NVMeTask(move(promise));
	task->nvmeVerb = static_cast<NVMeVerb>(Op);
	task->bufferSize = bufferSize;
	task->buffer = buffer;
	task->addr = addr;

	size_t retries = 0;
	while (!submitTask(task)) {
		if (retries == 1000) {
			promise.set_exception(make_exception_ptr(runtime_error("Failed to submit a task")));
			delete task;
		}
		this_thread::yield();
		++retries;
	}
	return future;
}

void NVMeManager::CompletionClbk(void* arg, const spdk_nvme_cpl* completion)
{
	auto task = static_cast<NVMeTask*>(arg);
	if (spdk_nvme_cpl_is_error(completion)) {
		task->prom.set_exception(make_exception_ptr(runtime_error(
				string("Operation failed. Code type: ") + to_string(completion->status.sct) + ". Code: " +
				to_string(completion->status.sc))));
		delete task;
		throw runtime_error(string("Operation failed. Code type: ") + to_string(completion->status.sct) + ". Code: " +
							to_string(completion->status.sc));
	}
	else {
		task->prom.set_value();
	}
	delete task;
}

void NVMeManager::AttachClbk(void* context, const spdk_nvme_transport_id* transportId, spdk_nvme_ctrlr* ctrlr, const spdk_nvme_ctrlr_opts*)
{
	printf("Attach callback for %s", transportId->traddr);
	spdk_pci_addr pci_addr;
	if (spdk_pci_addr_parse(&pci_addr, transportId->traddr)) {
		throw runtime_error("Cant obtain PCIe address");
	}

	auto pci_dev = spdk_nvme_ctrlr_get_pci_device(ctrlr);
	if (!pci_dev) {
		throw runtime_error("Cant obtain PCIe device");
	}
	auto pci_id = spdk_pci_device_get_id(pci_dev);

	printf("Attached to NVMe Controller at %s [%04x:%04x]", transportId->traddr, pci_id.vendor_id, pci_id.device_id);

	NVMeController ctrlEntry;
	ctrlEntry.ctrlr = ctrlr;
	for (auto namespaceId = spdk_nvme_ctrlr_get_first_active_ns(ctrlr);
		 namespaceId != 0; namespaceId = spdk_nvme_ctrlr_get_next_active_ns(ctrlr, namespaceId)) {
		auto ns = spdk_nvme_ctrlr_get_ns(ctrlr, namespaceId);
		if (ns != NULL && spdk_nvme_ns_is_active(ns)) {
			ctrlEntry.namespaces.emplace_back(NVMeNamespace{ns});
		}
	}
	ctrlEntry.transportName = "trid:";
	ctrlEntry.transportName += transportId->traddr;
	static_cast<NVMeManager*>(context)->controller = ctrlEntry;
}

NVMeManager::~NVMeManager()
{
	shouldStop = true;
	loopService.join();
	spdk_nvme_detach(controller.ctrlr);;
}

uint64_t NVMeManager::GetSize() const
{
	return controller.namespaces[0].nsSize;
}

unsigned int NVMeManager::GetBlockSize() const
{
	return controller.namespaces[0].nsBlockSize;
}

NVMeManager::NVMeTask::NVMeTask(promise<void> promParam) : prom(move(promParam))
{
}
