#define NONIUS_RUNNER
#include "../Tests/nonius/nonius.h++"

#include <iostream>
#include <numeric>
#include <type_traits>
#include <vector>
//#include <format>
#include <boost/algorithm/string/join.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_string.hpp>

namespace Join
{
template <template <typename, typename> class Container, typename Value, typename Allocator = std::allocator<Value>,
          std::enable_if_t<std::is_same<Value, std::string>::value>* = nullptr>
std::string join1(const Container<Value, std::allocator<Value>>& data, const std::string& sep)
{
    std::string ret_val;
    std::for_each(data.cbegin(), data.cend(), [&ret_val, &sep](const auto& item) {
        ret_val += ret_val.empty() ? "" : sep;
        ret_val += item;
    });
    return ret_val;
}

template <template <typename, typename> class Container, typename Value, typename Allocator = std::allocator<Value>,
          std::enable_if_t<std::is_same<Value, std::string>::value>* = nullptr>
std::string join4(const Container<Value, std::allocator<Value>>& data, const std::string& sep)
{
    std::string ret_val;
    ret_val.reserve(std::accumulate(data.cbegin(), data.cend(), 0, [](auto init, const auto& item) { return init + item.size(); }) +
                    data.size() * sep.size());
    std::for_each(data.cbegin(), data.cend(), [&ret_val, &sep](const auto& item) {
        ret_val += ret_val.empty() ? "" : sep;
        ret_val += item;
    });
    return ret_val;
}

template <template <typename, typename> class Container, typename Value, typename Allocator = std::allocator<Value>,
          std::enable_if_t<std::is_same<Value, std::string>::value>* = nullptr>
std::string join5(const Container<Value, std::allocator<Value>>& data, const std::string& sep)
{
    return boost::algorithm::join(data, sep);
}

template <template <typename, typename> class Container, typename Value, typename Allocator = std::allocator<Value>,
          std::enable_if_t<std::is_same<Value, std::string>::value>* = nullptr>
std::string join6(const Container<Value, std::allocator<Value>>& data, const std::string& sep)
{
    using boost::spirit::karma::generate_delimited;
    using boost::spirit::karma::string;
    std::string generated;
    generated.reserve(std::accumulate(data.cbegin(), data.cend(), 0, [](auto init, const auto& item) { return init + item.size(); }) +
                      data.size() * sep.size());
    std::back_insert_iterator<std::string> sink(generated);
    if(generate_delimited(sink, string << *(string), sep, data))
    {
        return generated;
    }

    return {};
}

template <template <typename, typename> class Container, typename Value, typename Allocator = std::allocator<Value>,
    std::enable_if_t<std::is_same<Value, std::string>::value>* = nullptr>
std::string join7(const Container<Value, std::allocator<Value>>& data, const std::string& sep)
{
    auto first = data.cbegin();
    auto last = data.cend();
    std::stringstream ret_val;
    if(first != last)
    {
        ret_val << *first;
        while(++first != last)
        {
            ret_val << sep;
            ret_val << *first;
        }
    }
    return ret_val.str();
}

} // namespace Join

namespace VariadicJoin
{
template <typename Start, typename... Args>
std::string join1(const std::string& sep, Start&& arg, Args&&... args)
{
    return arg + ((sep + args) + ...);
}

// for_each_in_tuple by Andy Prowl - https://stackoverflow.com/questions/16387354/template-tuple-calling-a-function-on-each-element
namespace detail
{
template <int... Is>
struct seq
{
};

template <int N, int... Is>
struct gen_seq : gen_seq<N - 1, N - 1, Is...>
{
};

template <int... Is>
struct gen_seq<0, Is...> : seq<Is...>
{
};

template <typename T, typename F, int... Is>
void for_each(T&& t, F f, seq<Is...>)
{
    auto l = {(f(std::get<Is>(t)), 0)...};
}
} // namespace detail

template <typename... Ts, typename F>
void for_each_in_tuple(std::tuple<Ts...> const& t, F f)
{
    detail::for_each(t, f, detail::gen_seq<sizeof...(Ts)>());
}

template <typename... Args>
std::string join2(const std::string& sep, Args&&... args)
{
    std::string ret_val;
    size_t length = 0;
    for_each_in_tuple(std::forward_as_tuple(args...), [&length](const auto& item) { length += item.size(); });
    length += sizeof...(args) * sep.size();
    ret_val.reserve(length);

    for_each_in_tuple(std::forward_as_tuple(args...), [&ret_val, &sep](const auto& item) {
        ret_val += ret_val.empty() ? "" : sep;
        ret_val += item;
    });
    return ret_val;
}

// template <typename ...Args>
// std::string join2(const std::string& sep, Args&&... args)
//{
//	std::string format;
//	constexpr auto size = sizeof...(Args);
//	format.reserve(size * 2 + size *sep.size());

//	for (auto i = 0; i < size; ++i)
//	{
//		format += "{}";
//		format += sep;
//	}
//	return std::format(format, args...);
//}
} // namespace VariadicJoin

static const std::vector<std::string> input{"Hello123456789012345678901234567890",   "Kitty123456789012345678901234567890",
                                            "in123456789012345678901234567890",      "the123456789012345678901234567890",
                                            "strange123456789012345678901234567890", "world123456789012345678901234567890"};

NONIUS_BENCHMARK("stl for_each", [](nonius::chronometer meter) { meter.measure([] { return Join::join1(input, ","); }); });
NONIUS_BENCHMARK("stl for_each with exact reserve",
                 [](nonius::chronometer meter) { meter.measure([] { return Join::join4(input, ","); }); });
NONIUS_BENCHMARK("boost join", [](nonius::chronometer meter) { meter.measure([] { return Join::join5(input, ","); }); });
NONIUS_BENCHMARK("boost karma", [](nonius::chronometer meter) { meter.measure([] { return Join::join6(input, ","); }); });
NONIUS_BENCHMARK("stringstream", [](nonius::chronometer meter) { meter.measure([] { return Join::join7(input, ","); }); });

NONIUS_BENCHMARK("Variadic fold join", [](nonius::chronometer meter) {
    meter.measure([] {
        return VariadicJoin::join1(",", "Hello123456789012345678901234567890", "Kitty123456789012345678901234567890",
                                   "in123456789012345678901234567890", "the123456789012345678901234567890",
                                   "strange123456789012345678901234567890", "world123456789012345678901234567890");
    });
});

NONIUS_BENCHMARK("Variadic tuple join", [](nonius::chronometer meter) {
    using namespace std::string_literals;
    meter.measure([] {
        return VariadicJoin::join2(","s, "Hello123456789012345678901234567890"s, "Kitty123456789012345678901234567890"s,
                                   "in123456789012345678901234567890"s, "the123456789012345678901234567890"s,
                                   "strange123456789012345678901234567890"s, "world123456789012345678901234567890"s);
    });
});

// int main()
//{
//    using namespace std::string_literals;
//    std::cout << Join::join1(input, ",") << std::endl;
//    std::cout << Join::join4(input, ",") << std::endl;
//    std::cout << Join::join5(input, ",") << std::endl;
//    std::cout << Join::join6(input, ",") << std::endl;
//    std::cout << VariadicJoin::join1(",", "Hello", "Kitty", "in", "the", "strange", "world") << std::endl;
//    std::cout << VariadicJoin::join2(",", "Hello"s, "Kitty"s, "in"s, "the"s, "strange"s, "world"s) << std::endl;
//}
