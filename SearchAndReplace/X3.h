#pragma once
#include <boost/spirit/home/x3.hpp>

namespace x3 = boost::spirit::x3;

struct append
{
    std::string& out;
    void do_append(char const ch) const { out += ch; }
    void do_append(std::string const& s) const { out += s; }
    template <typename It>
    void do_append(boost::iterator_range<It> const& r) const
    {
        out.append(r.begin(), r.end());
    }
    template <typename Ctx>
    void operator()(Ctx& ctx) const
    {
        do_append(_attr(ctx));
    }
};
std::string expandTokensX3(const std::string& input, x3::standard::symbols<char const*> const& symbols);
