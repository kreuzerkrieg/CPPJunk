#pragma once

#include <type_traits>
#include <boost/lockfree/queue.hpp>

template<typename T>
class Queue
{
	using StorageType = typename std::remove_reference<typename std::remove_cv<typename std::remove_pointer<T>::type>::type>::type;
	boost::lockfree::queue<StorageType*> items;

public:
	Queue(size_t size = 0) : items(size)
	{
	}

	template<typename ValueType, std::enable_if_t<!std::is_pointer<ValueType>::value>* = nullptr>
	bool Push(ValueType item)
	{
		auto tmp = new StorageType(std::forward<ValueType>(item));
		return items.push(tmp);
	}

	template<typename ValueType, std::enable_if_t<std::is_pointer<ValueType>::value>* = nullptr>
	bool Push(ValueType item)
	{
		return items.push(item);
	}

	template<typename ValueType, std::enable_if_t<!std::is_pointer<ValueType>::value>* = nullptr>
	bool Pop(ValueType& item)
	{
		StorageType* tmp;
		auto res = items.pop(tmp);
		if (res) {
			item = std::move(*tmp);
			delete tmp;
		}
		return res;
	}

	template<typename ValueType, std::enable_if_t<std::is_pointer<ValueType>::value>* = nullptr>
	bool Pop(ValueType& item)
	{
		return items.pop(item);
	}

	size_t GetCount() const
	{
		return 5ul; //TODO EWZ
	}
};
