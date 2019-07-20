//#include <unordered_map>
//#include <string>
//#include <iostream>
//#include <chrono>
//#include <boost/spirit/include/qi.hpp>
//#include <boost/algorithm/string.hpp>
//#include <boost/algorithm/searching/knuth_morris_pratt.hpp>
//#include <boost/algorithm/searching/boyer_moore_horspool.hpp>
//#include <boost/algorithm/searching/boyer_moore.hpp>
//#include <assert.h>
#include "X3.h"

std::string expandTokensX3(const std::string& input, x3::standard::symbols<char const*> const& symbols)
{
    std::string retVal;
    retVal.reserve(input.size() * 2);
    append appender{retVal};

    auto beg = input.cbegin();
    auto end = input.cend();

    auto rule = *(symbols[appender] | boost::spirit::x3::standard::char_[appender]);

    if(!boost::spirit::x3::parse(beg, end, rule))
        retVal = input;

    return retVal;
}