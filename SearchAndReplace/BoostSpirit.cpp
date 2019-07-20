#include "BoostSpirit.h"

std::string expandTokensBoostSpirit(const std::string& input, bsq::symbols<char, std::string> const& symbols)
{
    std::string retVal;
    retVal.reserve(input.size() * 2);

    auto beg = input.cbegin();
    auto end = input.cend();

    if(!bsq::parse(beg, end, *(symbols | +(bsq::char_ - symbols)), retVal))
        retVal = input;

    return retVal;
}
