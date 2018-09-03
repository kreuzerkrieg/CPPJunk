/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>
#include <experimental/optional>

#include <grpcpp/grpcpp.h>
#include <thread>
#include <chrono>

#include <rpc.grpc.pb.h>
//#include "Watcher.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using namespace etcdserverpb;

class EtcdClient
{
public:
	EtcdClient(std::shared_ptr<Channel> channel) : stub(channel)
	{}

	void Put(const std::string& key, const std::string& value)
	{
		PutRequest put_request;
		put_request.set_key(key);
		put_request.set_value(value);
		put_request.set_prev_kv(true);

		ClientContext context;
		PutResponse put_response;
		Status status = stub.Put(&context, put_request, &put_response);

		if (!status.ok()) {
			throw std::runtime_error(status.error_message());
		}
	}

	std::experimental::optional<std::string> Get(const std::string& key)
	{
		RangeRequest range_request;
		range_request.set_key(key);

		ClientContext context;
		RangeResponse range_response;
		Status status = stub.Range(&context, range_request, &range_response);
		if (status.ok()) {
			if (range_response.kvs().size() == 0) {
				std::cout << "Key not found" << std::endl;
				return std::experimental::optional<std::string>();
			}
			return range_response.kvs().Get(0).value();
		}
		else {
			throw std::runtime_error(status.error_message());
		}
	}

	std::map<std::string, std::string> GetList(const std::string& key)
	{
		RangeRequest range_request;
		range_request.set_key(key);
		range_request.set_range_end(key + "\xFF");

		ClientContext context;
		RangeResponse range_response;
		Status status = stub.Range(&context, range_request, &range_response);
		if (status.ok()) {
			if (range_response.kvs().size() == 0) {
				throw std::runtime_error(status.error_message());
			}
			std::cout << "KSV size: " << range_response.kvs().size() << std::endl;
			std::map<std::string, std::string> retVal;
			std::transform(std::begin(range_response.kvs()), std::end(range_response.kvs()), std::inserter(retVal, retVal.end()),
						   [](auto& item) { return std::make_pair(item.key(), item.value()); });
			return retVal;
		}
		else {
			throw std::runtime_error(status.error_message());
		}
	}

	void PutTxn(const std::vector<std::pair<std::string, std::string>>& kvs)
	{
		etcdserverpb::TxnRequest txn_request;

		for (const auto& kv:kvs) {
			AddToTxn(kv.first, kv.second, txn_request);
		}
		ClientContext context;
		TxnResponse reply;
		auto status = stub.Txn(&context, txn_request, &reply);
		if (!status.ok()) {
			throw std::runtime_error(status.error_message());
		}
	}

	void AddToTxn(const std::string& key, const std::string& value, TxnRequest& txn) const
	{
		{    // Basic setup
			Compare* compare = txn.add_compare();
			compare->set_result(Compare_CompareResult_EQUAL);
			compare->set_target(Compare_CompareTarget_VERSION);
			compare->set_key(key);
			compare->set_version(0);
		}
		{    // Setup success
			std::unique_ptr<PutRequest> put_request(new PutRequest());
			put_request->set_key(key);
			put_request->set_value(value);
			put_request->set_prev_kv(true);
			put_request->set_lease(0);
			RequestOp* req_success = txn.add_success();
			req_success->set_allocated_request_put(put_request.release());

			std::unique_ptr<RangeRequest> get_request(new RangeRequest());
			get_request->set_key(key);
			req_success = txn.add_success();
			req_success->set_allocated_request_range(get_request.release());
		}
		{    // Setup failure
			std::unique_ptr<PutRequest> put_request(new PutRequest());
			put_request->set_key(key);
			put_request->set_value(value);
			put_request->set_prev_kv(true);
			put_request->set_lease(0);
			RequestOp* req_failure = txn.add_failure();
			req_failure->set_allocated_request_put(put_request.release());

			std::unique_ptr<RangeRequest> get_request(new RangeRequest());
			get_request->set_key(key);
			req_failure = txn.add_failure();
			req_failure->set_allocated_request_range(get_request.release());
		}
	}

	std::vector<std::pair<std::string, std::string>> GetTxn(const std::vector<std::string>& kvs)
	{
		etcdserverpb::TxnRequest txn_request;

		for (const auto& kv:kvs) {
			AddToTxn1(kv, txn_request);
		}
		ClientContext context;
		TxnResponse reply;
		auto status = stub.Txn(&context, txn_request, &reply);
		if (status.ok()) {
			if (reply.responses().size() == 0) {
				std::cout << "Key not found" << std::endl;
				return {};
			}
			std::vector<std::pair<std::string, std::string>> retVal;
			for (const auto& response: reply.responses()) {
				for (const auto& kv:response.response_range().kvs()) {
					retVal.emplace_back(std::make_pair(kv.key(), kv.value()));
				}
			}
			return retVal;
		}
		else {
			throw std::runtime_error(status.error_message());
		}
	}

	void AddToTxn1(const std::string& key, TxnRequest& txn) const
	{
		{    // Basic setup
			Compare* compare = txn.add_compare();
			compare->set_result(Compare_CompareResult_EQUAL);
			compare->set_target(Compare_CompareTarget_VERSION);
			compare->set_key(key);
			compare->set_version(0);
		}
		{    // Setup success
			std::unique_ptr<RangeRequest> get_request(new RangeRequest());
			get_request->set_key(key);
			RequestOp* req_success = txn.add_success();
			req_success->set_allocated_request_range(get_request.release());
		}
		{    // Setup failure
			std::unique_ptr<RangeRequest> get_request(new RangeRequest());
			get_request->set_key(key);
			RequestOp* req_failure = txn.add_failure();
			req_failure->set_allocated_request_range(get_request.release());
		}
	}

private:
	KV::Stub stub;
};

using namespace grpc;

class Watcher
{
public:
	using Callback = std::function<void(const std::string&, const std::string&)>;

	struct WatchPackage
	{
		std::unique_ptr<ClientAsyncReaderWriter<WatchRequest, WatchResponse>> stream;
		std::unique_ptr<WatchResponse> reply {std::make_unique<WatchResponse>()};
		std::unique_ptr<ClientContext> context {std::make_unique<ClientContext>()};
		Callback callback;
		std::string key;
		bool isRecursive = false;
		const std::string cancelationPrefix = "\x7FStopWatching\x80";

		WatchPackage() = default;

		WatchPackage(WatchPackage&&) = default;

		WatchPackage& operator=(WatchPackage&&) = default;

		void Finish()
		{
			if (stream) {
				Status status;
				stream->WritesDone((void*) "finish");
				std::cout << status.error_details() << std::endl;
			}
		}

		~WatchPackage()
		{
			//			if (stream) {
			//				Status status;
			//				stream->Finish(&status, (void*)"finish");
			//			}
		}
	};

	Watcher(std::shared_ptr<Channel> channel) : watchStub(channel)
	{
		eventPoller = std::thread([this]() { WaitForEvent(); });
	}

	~Watcher()
	{
		// lock temporaries and callbacks
		for (auto& package:temporary) {
			package.second.Finish();
		}
		for (auto& package:callbacks) {
			package.second.Finish();
		}

		//completionQueue.Shutdown();

		eventPoller.join();
		temporary.clear();
		callbacks.clear();
	}

	void AddWatch(const std::string& key, Callback callback)
	{
		AddWatch(key, callback, false);
	}

	void AddWatches(const std::string& key, Callback callback)
	{
		AddWatch(key, callback, true);
	}

	void CancelWatch(const std::string& key)
	{
		// lock temporaries and callbacks
		temporary.erase(creationPrefix + key);
		callbacks.erase(key);
	}

private:
	void AddWatch(const std::string& key, Callback callback, bool isRecursive)
	{
		auto insertionResult = temporary.emplace(creationPrefix + key, WatchPackage());
		WatchPackage& package = insertionResult.first->second;

		package.callback = std::move(callback);
		package.key = key;
		package.isRecursive = isRecursive;
		package.stream = watchStub.AsyncWatch(package.context.get(), &completionQueue, (void*) insertionResult.first->first.c_str());
	}

	void BuildWatchRequest(WatchPackage& package)
	{
		WatchCreateRequest watch_create_req;
		watch_create_req.set_key(package.key);
		if (package.isRecursive) {
			watch_create_req.set_range_end(package.key + "\xFF");
		}

		WatchRequest watch_req;
		watch_req.mutable_create_request()->CopyFrom(watch_create_req);
		package.stream->Write(watch_req, nullptr);
		package.stream->Read(package.reply.get(), (void*) package.key.c_str());
	}

	void WaitForEvent()
	{
		void* got_tag;
		bool ok = false;

		while (completionQueue.Next(&got_tag, &ok)) {
			if (ok == false) {
				throw std::runtime_error("Queue polling failed");
			}
			else if (got_tag == nullptr) {
				continue;
			}
			else if (got_tag == "finish") {
				Status status;
				callbacks.begin()->second.stream->Finish(&status, (void*) nullptr);
				completionQueue.Shutdown();
			}
			else if (memcmp(got_tag, cancelationPrefix.c_str(), cancelationPrefix.size()) == 0) {
				//break;
			}
			else if (memcmp(got_tag, creationPrefix.c_str(), creationPrefix.size()) == 0) {
				// lock here the temporaries
				auto tag = std::string(reinterpret_cast<char*>(got_tag));
				auto findIt = temporary.find(tag);
				if (findIt == temporary.end()) {
					throw std::runtime_error("Key \"" + tag + "\"not found");
				}
				auto insertionResult = callbacks.emplace(findIt->second.key, std::move(findIt->second));
				temporary.erase(tag);
				BuildWatchRequest(insertionResult.first->second);
			}
			else {
				// lock here the callbacks
				auto tag = std::string(reinterpret_cast<char*>(got_tag));
				auto findIt = callbacks.find(tag);
				if (findIt == callbacks.end()) {
					throw std::runtime_error("Key \"" + tag + "\"not found");
				}

				if (findIt->second.reply->events_size()) {
					ParseResponse(*findIt->second.reply, findIt->second.callback);
				}
				findIt->second.stream->Read(findIt->second.reply.get(), got_tag);
			}
		}
	}

	void ParseResponse(WatchResponse& reply, Callback& callback)
	{
		for (int i = 0; i < reply.events_size(); ++i) {
			auto event = reply.events(i);
			callback(event.kv().key(), event.kv().value());
		}
	}

	Watch::Stub watchStub;
	CompletionQueue completionQueue;
	std::unordered_map<std::string, WatchPackage> callbacks;
	std::unordered_map<std::string, WatchPackage> temporary;
	std::thread eventPoller;
	const std::string creationPrefix = "\x7Fcreate\x80_";
	const std::string cancelationPrefix = "\x7FStopWatching\x80";
};

class NewWatcher
{
public:
	using Callback = std::function<void(const std::string&, const std::string&)>;

	struct WatchPackage
	{
		enum class CallStatus
		{
			Create, Listening, Destroy, Finally
		};
		std::unique_ptr<ClientAsyncReaderWriter<WatchRequest, WatchResponse>> stream;
		WatchResponse reply;
		ClientContext context;
		Callback callback;
		std::string key;
		CallStatus callStatus = CallStatus::Create;
		bool isRecursive = false;
		Status status;
	};

	NewWatcher(std::shared_ptr<Channel> channel) : watchStub(channel)
	{ eventPoller = std::thread([this]() { WaitForEvent(); }); }

	~NewWatcher()
	{
		stopping = true;
		for (auto& package : watches) {
			Destroy(&(package.second));
		}
		//completionQueue.Shutdown();
		eventPoller.join();
	}

	void AddWatch(const std::string& key, Callback callback)
	{
		AddWatch(key, callback, false);
	}

	void AddWatches(const std::string& key, Callback callback)
	{
		AddWatch(key, callback, true);
	}

	void CancelWatch(const std::string& key)
	{
		// lock temporaries and callbacks
		Destroy(&watches[key]);
	}

private:
	void AddWatch(const std::string& key, Callback callback, bool isRecursive)
	{
		//auto res = watches.emplace(key, WatchPackage());
		auto& res = watches[key];
		WatchPackage* package = &(res);

		package->callback = std::move(callback);
		package->key = key;
		package->isRecursive = isRecursive;
		package->callStatus = WatchPackage::CallStatus::Create;
		package->stream = watchStub.AsyncWatch(&package->context, &completionQueue, (void*) package);
	}

	void WaitForEvent()
	{
		void* tag;
		bool ok = false;
		grpc::ServerCompletionQueue::NextStatus nextStatus;
		while (nextStatus = completionQueue.AsyncNext(&tag, &ok, std::chrono::system_clock::now() + std::chrono::seconds(1))) {
			if (watches.empty() && stopping) {
				completionQueue.Shutdown();
				break;
			}

			if (nextStatus == grpc::ServerCompletionQueue::TIMEOUT) {
				continue;
			}
			if (ok == false && stopping) {
				break;
			}
			else if (ok == false && !stopping) {
				throw std::runtime_error("Queue polling failed");
			}
			else if (tag == nullptr) {
				continue;
			}
			else {
				auto package = static_cast<WatchPackage*>(tag);
				switch (package->callStatus) {
				case WatchPackage::CallStatus::Create:
					Create(package);
					break;
				case WatchPackage::CallStatus::Destroy:
					Destroy(package);
					break;
				case WatchPackage::CallStatus::Finally:
					Finally(package);
					break;
				case WatchPackage::CallStatus::Listening:
					Notify(package);
					break;
				default:
					throw std::runtime_error("Error");
				}
			}
		}
	}

	void Create(WatchPackage* package)
	{
		WatchCreateRequest watch_create_req;
		watch_create_req.set_key(package->key);
		if (package->isRecursive) {
			watch_create_req.set_range_end(package->key + "\xFF");
		}

		WatchRequest watch_req;
		watch_req.mutable_create_request()->CopyFrom(watch_create_req);
		package->callStatus = WatchPackage::CallStatus::Listening;
		package->stream->Write(watch_req, nullptr);
		package->stream->Read(&package->reply, (void*) package);
	}

	void Destroy(WatchPackage* package)
	{
		package->callStatus = WatchPackage::CallStatus::Finally;
		package->stream->WritesDone((void*) package);
		//package->stream->Finish(&(package->status), (void*) package);
		//watches.erase(package->key);
	}

	void Notify(WatchPackage* package)
	{
		for (int i = 0; i < package->reply.events_size(); ++i) {
			auto event = package->reply.events(i);
			package->callback(event.kv().key(), event.kv().value());
		}
		package->stream->Read(&package->reply, (void*) package);
	}

	void Finally(WatchPackage* package)
	{
		watches.erase(package->key);
	}

	Watch::Stub watchStub;
	CompletionQueue completionQueue;
	std::unordered_map<std::string, WatchPackage> watches;
	std::thread eventPoller;
	bool stopping = false;
};

class IObserver
{
public:
	virtual ~IObserver() = default;

	virtual void OnConfigChanged(const string& key) = 0;
};

class ETCDObserver : public IObserver
{
	void OnConfigChanged(const string& key)
	{
		EtcdClient client(grpc::CreateChannel("localhost:2379", grpc::InsecureChannelCredentials()));
		std::cout << key << " key has a new value " << client.Get(key).value() << std::endl;
	}
};

int main(int argc, char** argv)
{
	// Instantiate the client. It requires a channel, out of which the actual RPCs
	// are created. This channel models a connection to etcd server
	// We indicate that the channel isn't authenticated
	// (use of InsecureChannelCredentials()).
	EtcdClient client(grpc::CreateChannel("localhost:2379", grpc::InsecureChannelCredentials()));

	std::vector<std::pair<std::string, std::string>> kvs {{"Hello",  "World"},
														  {"Hello1", "Jopa"},
														  {"Jopa",   "Hello"}};
	client.PutTxn(kvs);
	//	client.Put("Hello1", "World1");
	//	client.Put("Hello3", "World!");
	//	client.Put("Helloo", "World1");
	//	client.Put("Hellow", "World!");
	//	client.Put("Hellooooow", "World!");
	//	std::cout << "Recieved : " << client.Get("Hello").value() << std::endl;
	//	for (const auto& entry:client.GetList("Hello")) {
	//		std::cout << entry.first << " : " << entry.second << std::endl;
	//	}

	for (const auto& entry:client.GetTxn({"Hello", "Jopa", "Hello1"})) {
		std::cout << entry.first << " : " << entry.second << std::endl;
	}
	//	NewWatcher watcher(grpc::CreateChannel("localhost:2379", grpc::InsecureChannelCredentials()));
	//	auto lambda = [](const std::string& key, const std::string& value) {
	//		std::cout << key << " key has a new value " << value << std::endl;
	//	};
	//	class MiniObserver
	//	{
	//	public:
	//		void OnConfigChanged(const string& key)
	//		{
	//			std::cout << "Callback called for key " << key << std::endl;
	//		}
	//	};
	//	MiniObserver observer;
	//	//watcher.AddWatch("blah", std::bind(&MiniObserver::OnConfigChanged, &observer, std::placeholders::_1));
	//	std::this_thread::sleep_for(std::chrono::seconds(1));
	//	//watcher.AddWatch("Hello3", lambda);
	//	client.Put("Hello", "Kitty");
	//	//client.Put("Hello1", "Kitty!");
	//	//client.Put("Hello", "Kitty");
	//	//client.Put("Hello", "Kitty");
	//	//	client.Put("Hello3", "Kitty!!!");
	//	//	client.Put("Hello3", "Kitty!!!");
	//	/*while (true) {
	//		//client.Put("Hello", "Kitty");
	//		//client.Put("Hello3", "Kitty!!!");
	//		std::this_thread::yield();
	//	}*/
	//	watcher.CancelWatch("blah");
	//	std::this_thread::sleep_for(std::chrono::seconds(1));
	//	client.Put("Hello", "Kitty");
	//	//	watcher.AddWatch("Hello", lambda);
	//	//	std::this_thread::sleep_for(std::chrono::seconds(1));
	//	//	client.Put("Hello", "Kitty");
	std::this_thread::sleep_for(std::chrono::seconds(3));
	return 0;
}
