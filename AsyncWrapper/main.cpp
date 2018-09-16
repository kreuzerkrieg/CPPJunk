#include <iostream>
#include <vector>
#include <future>

template<typename ValueType>
class Asyncronizer
{
public:
	template<typename Type, typename ...Args, std::enable_if_t<std::is_void<Type>::value>* = nullptr>
	std::future<void> call(Type (ValueType::*mf)(Args...), Args&& ... args)
	{
		std::promise<void> promise;
		auto future = promise.get_future();
		std::thread([promise {std::move(promise)}, &mf, this](Args&& ... args) mutable {
			try {
				(object.*mf)(std::forward<Args>(args)...);
				promise.set_value();
			}
			catch (...) {
				promise.set_exception(std::current_exception());
			}
		}).detach();
		return future;
	}

	template<typename Type, typename ...Args, std::enable_if_t<!std::is_void<Type>::value>* = nullptr>
	std::future<Type> call(Type (ValueType::*mf)(Args...), Args&& ... args)
	{
		std::promise<Type> promise;
		auto future = promise.get_future();
		std::thread([promise {std::move(promise)}, &mf, this](Args&& ... args) mutable {
			try {
				auto result = (object.*mf)(std::forward<Args>(args)...);
				promise.set_value(result);
			}
			catch (...) {
				promise.set_exception(std::current_exception());
			}
		}).detach();
		return future;
	}

	template<typename Type, typename ...Args, std::enable_if_t<std::is_void<Type>::value>* = nullptr>
	std::future<void> call(Type (ValueType::*mf)(Args...) const, Args&& ... args)
	{
		std::promise<void> promise;
		auto future = promise.get_future();
		std::thread([promise {std::move(promise)}, &mf, this](Args&& ... args) mutable {
			try {
				(object.*mf)(std::forward<Args>(args)...);
				promise.set_value();
			}
			catch (...) {
				promise.set_exception(std::current_exception());
			}
		}).detach();
		return future;
	}

	template<typename Type, typename ...Args, std::enable_if_t<!std::is_void<Type>::value>* = nullptr>
	std::future<Type> call(Type (ValueType::*mf)(Args...) const, Args&& ... args)
	{
		std::promise<Type> promise;
		auto future = promise.get_future();
		std::thread([promise {std::move(promise)}, &mf, this](Args&& ... args) mutable {
			try {
				auto result = (object.*mf)(std::forward<Args>(args)...);
				promise.set_value(result);
			}
			catch (...) {
				promise.set_exception(std::current_exception());
			}
		}).detach();
		return future;
	}

private:
	ValueType object;
};

class Test
{
public:
	std::vector<uint8_t> Test1()
	{
		return {};
	}

	uint64_t Test2(uint64_t arg1, uint32_t arg2)
	{
		return 42;
	}

	std::string Test3(double num) const
	{
		return std::to_string(num);
	}

	void Test4()
	{
	}

	int Test5()
	{
		return 42;
	}

private:

	int foo()
	{
		return 1;
	}

	double bar()
	{
		return 1.;
	}
};

int main()
{
	//auto result1 = Asyncronizer<Test>().call(&Test::Test1);
	auto result2 = Asyncronizer<Test>().call(&Test::Test2, 1ul, 2u);
	//auto result3 = Asyncronizer<Test>().call(&Test::Test3, 3.14);
//	auto result4 = Asyncronizer<Test>().call(&Test::Test4);
//	auto result5 = Asyncronizer<Test>().call(&Test::Test5);

	return 0;
}