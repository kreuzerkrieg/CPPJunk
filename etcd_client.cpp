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

#include "rpc.grpc.pb.h"
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

private:
	KV::Stub stub;
	TxnRequest txn_request;
};

using namespace grpc;

class Watcher
{
public:
	using Callback = std::function<void(const std::string&, const std::string&)>;
	enum class EventType : uint8_t
	{
		Create, Delete, Update, EventTypeMAX
	};

	class WatchPackage
	{

	};

	Watcher(std::shared_ptr<Channel> channel) : watchStub(channel)
	{
		stream = watchStub.AsyncWatch(&context, &completionQueue, (void*) "create");
		eventPoller = std::thread([this]() { WaitForEvent(); });
	}

	void AddWatch(const std::string& key, Callback callback)
	{
		AddWatch(key, callback, false);
	}

	void AddWatches(const std::string& key, Callback callback)
	{
		AddWatch(key, callback, true);
	}

	void RemoveWatch(const std::string& key)
	{
		WatchRequest watch_req;
		WatchCancelRequest watch_create_req;
		//		watch_create_req.set_key(key);
		//		//		watch_create_req.set_range_end(key + "\xFF");
		//		//		watch_create_req.set_prev_kv(true);
		//		//		watch_create_req.set_start_revision(parameters.revision);
		//
		//		watch_req.mutable_create_request()->CopyFrom(watch_create_req);
		//		stream->Write(watch_req, (void*) "write");
		//		WatchResponse reply;
		//		stream->Read(&reply, (void*) this);
		//		callbacks.emplace(key, callback);
	}

private:
	void AddWatch(const std::string& key, Callback callback, bool isRecursive)
	{
		auto insertionResult = callbacks.emplace(key, callback);
		if (!insertionResult.second) {
			throw std::runtime_error("Event handle already exist.");
		}
		WatchRequest watch_req;
		WatchCreateRequest watch_create_req;
		watch_create_req.set_key(key);
		if (isRecursive) {
			watch_create_req.set_range_end(key + "\xFF");
		}

		watch_req.mutable_create_request()->CopyFrom(watch_create_req);
		stream->Write(watch_req, (void*) insertionResult.first->first.c_str());

		stream->Read(&reply, (void*) insertionResult.first->first.c_str());
	}

	void WaitForEvent()
	{
		void* got_tag;
		bool ok = false;

		while (completionQueue.Next(&got_tag, &ok)) {
			if (ok == false) {
				break;
			}
			if (got_tag == (void*) "writes done") {
				// Signal shutdown
			}
			else if (got_tag == (void*) "create") {
			}
			else if (got_tag == (void*) "write") {
			}
			else {

				auto tag = std::string(reinterpret_cast<char*>(got_tag));
				auto findIt = callbacks.find(tag);
				if (findIt == callbacks.end()) {
					throw std::runtime_error("Key \"" + tag + "\"not found");
				}

				if (reply.events_size()) {
					reply.
					ParseResponse(findIt->second);
				}
				stream->Read(&reply, got_tag);
			}
		}
	}

	void ParseResponse(Callback& callback)
	{
		for (int i = 0; i < reply.events_size(); ++i) {
			auto event = reply.events(i);
			auto key = event.kv().key();
			callback(event.kv().key(), event.kv().value());
		}
	}

	Watch::Stub watchStub;
	CompletionQueue completionQueue;
	ClientContext context;
	std::unique_ptr<ClientAsyncReaderWriter<WatchRequest, WatchResponse>> stream;
	WatchResponse reply;
	std::unordered_map<std::string, Callback> callbacks;
	std::thread eventPoller;

};

int main(int argc, char** argv)
{
	// Instantiate the client. It requires a channel, out of which the actual RPCs
	// are created. This channel models a connection to etcd server
	// We indicate that the channel isn't authenticated
	// (use of InsecureChannelCredentials()).
	EtcdClient client(grpc::CreateChannel("localhost:2379", grpc::InsecureChannelCredentials()));

	client.Put("Hello", "World");
	client.Put("Hello1", "World1");
	client.Put("Hello3", "World!");
	client.Put("Helloo", "World1");
	client.Put("Hellow", "World!");
	client.Put("Hellooooow", "World!");
	std::cout << "Recieved : " << client.Get("Hello").value() << std::endl;
	for (const auto& entry:client.GetList("Hello")) {
		std::cout << entry.first << " : " << entry.second << std::endl;
	}
	Watcher watcher(grpc::CreateChannel("localhost:2379", grpc::InsecureChannelCredentials()));
	auto lambda = [](const std::string& key, const std::string& value) {
		std::cout << key << " key has a new value " << value << std::endl;
	};
	watcher.AddWatches("Hello", lambda);
	//watcher.AddWatch("Hello3", lambda);
	client.Put("Hello", "Kitty");
	client.Put("Hello1", "Kitty!");
	client.Put("Hello3", "Kitty!!!");
	while (true) {
		std::this_thread::yield();
	}
	return 0;
}
