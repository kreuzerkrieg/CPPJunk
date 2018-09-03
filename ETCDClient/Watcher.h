#pragma once

#include <grpcpp/grpcpp.h>
#include "rpc.grpc.pb.h"

using namespace etcdserverpb;
using namespace grpc;

struct Response
{
};

struct Task
{
	Task()
	{};

	template<typename T>
	Task(T&& foo)
	{}

	void wait()
	{}
};

struct AsyncWatchResponse : public Response
{
	void set_error_code(int)
	{}

	void set_error_message(const std::string&)
	{}

	void ParseResponse(const WatchResponse&)
	{}
};

struct ActionParameters
{
	ActionParameters();

	bool withPrefix;
	int revision;
	int old_revision;
	int64_t lease_id;
	int ttl;
	std::string key;
	std::string value;
	std::string old_value;
	KV::Stub* kv_stub;
	Watch::Stub* watch_stub;
	Lease::Stub* lease_stub;
};

class Action
{
public:
	Action(ActionParameters params);

	Action()
	{};

	void waitForResponse();

protected:
	Status status;
	ClientContext context;
	CompletionQueue cq_;
	ActionParameters parameters;

};

class AsyncWatchAction : public Action
{
public:
	AsyncWatchAction(ActionParameters param) : Action(param)
	{
		isCancelled = false;
		stream = parameters.watch_stub->AsyncWatch(&context, &cq_, (void*) "create");

		WatchRequest watch_req;
		WatchCreateRequest watch_create_req;
		watch_create_req.set_key(parameters.key);
		watch_create_req.set_prev_kv(true);
		watch_create_req.set_start_revision(parameters.revision);

		if (parameters.withPrefix) {
			std::string range_end(parameters.key);
			int ascii = (int) range_end[range_end.length() - 1];
			range_end.back() = ascii + 1;
			watch_create_req.set_range_end(range_end);
		}

		watch_req.mutable_create_request()->CopyFrom(watch_create_req);
		stream->Write(watch_req, (void*) "write");
		stream->Read(&reply, (void*) this);
	}

	AsyncWatchResponse ParseResponse()
	{

		AsyncWatchResponse watch_resp;
		if (!status.ok()) {
			watch_resp.set_error_code(status.error_code());
			watch_resp.set_error_message(status.error_message());
		}
		else {
			watch_resp.ParseResponse(reply);
		}
		return watch_resp;
	}

	void waitForResponse()
	{
		void* got_tag;
		bool ok = false;

		while (cq_.Next(&got_tag, &ok)) {
			if (ok == false || (got_tag == (void*) "writes done")) {
				break;
			}
			if (got_tag == (void*) this) // read tag
			{
				if (reply.events_size()) {
					stream->WritesDone((void*) "writes done");
				}
				else {
					stream->Read(&reply, (void*) this);
				}
			}
		}
	}

	void waitForResponse(std::function<void(Response)> callback)
	{
		void* got_tag;
		bool ok = false;

		while (cq_.Next(&got_tag, &ok)) {
			if (ok == false) {
				break;
			}
			if (got_tag == (void*) "writes done") {
				isCancelled = true;
			}
			else if (got_tag == (void*) this) // read tag
			{
				if (reply.events_size()) {
					auto resp = ParseResponse();
					callback(resp);
				}
				stream->Read(&reply, (void*) this);
			}
		}
	}

	void CancelWatch()
	{
		if (isCancelled == false) {
			stream->WritesDone((void*) "writes done");
		}
	}

private:
	WatchResponse reply;
	std::unique_ptr<ClientAsyncReaderWriter<WatchRequest, WatchResponse>> stream;
	bool isCancelled;
};

class Watcher
{
public:
	Watcher(const std::string& uri, const std::string& key, std::function<void(Response)> callback)
	{
		std::shared_ptr<Channel> channel = grpc::CreateChannel(uri, grpc::InsecureChannelCredentials());
		watchServiceStub = Watch::NewStub(channel);

		doWatch(key, callback);
	}

	void Cancel()
	{
		call->CancelWatch();
		currentTask.wait();
	}

	~Watcher()
	{
		call->CancelWatch();
		currentTask.wait();
	}

protected:
	void doWatch(const std::string& key, std::function<void(Response)> callback)
	{
		ActionParameters params;
		params.key.assign(key);
		params.withPrefix = true;
		params.watch_stub = watchServiceStub.get();
		params.revision = 0;
		call.reset(new AsyncWatchAction(params));

		currentTask = Task([this, callback]() {
			return call->waitForResponse(callback);
		});
	}

	int index;
	std::function<void(Response)> callback;
	Task currentTask;
	std::unique_ptr<Watch::Stub> watchServiceStub;
	std::unique_ptr<KV::Stub> stub_;
	std::unique_ptr<AsyncWatchAction> call;
};