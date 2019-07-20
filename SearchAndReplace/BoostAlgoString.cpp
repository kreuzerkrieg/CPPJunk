#include "BoostAlgoString.h"
#include <boost/algorithm/searching/boyer_moore.hpp>
#include <boost/algorithm/searching/boyer_moore_horspool.hpp>
#include <boost/algorithm/searching/knuth_morris_pratt.hpp>
#include <boost/algorithm/string.hpp>

std::string expandTokensBoostReplaceAll(const std::string& input, const std::unordered_map<std::string, std::string>& tokens)
{
    std::string retVal(input);
    retVal.reserve(input.size() * 2);

    for(const auto& token : tokens)
    {
        boost::replace_all(retVal, token.first, token.second);
    }
    return retVal;
}

void expandTokensBoostBoyerMoore(std::string& input, const std::unordered_map<std::string, std::string>& tokens)
{
    input.reserve(input.size() * 2);
    for(const auto& token : tokens)
    {
        auto next = boost::algorithm::boyer_moore_search(input.cbegin(), input.cend(), token.first.begin(), token.first.end());
        while(next.second != input.cend())
        {
            input.replace(next.first, next.second, token.second);
            next = boost::algorithm::boyer_moore_search(input.cbegin(), input.cend(), token.first.begin(), token.first.end());
        }
    }
}

void expandTokensBoyerMooreHorspool(std::string& input, const std::unordered_map<std::string, std::string>& tokens)
{
    input.reserve(input.size() * 2);
    for(const auto& token : tokens)
    {
        auto next = boost::algorithm::boyer_moore_horspool_search(input.cbegin(), input.cend(), token.first.begin(), token.first.end());
        while(next.second != input.cend())
        {
            input.replace(next.first, next.second, token.second);
            next = boost::algorithm::boyer_moore_search(input.cbegin(), input.cend(), token.first.begin(), token.first.end());
        }
    }
}

void expandTokensBoostKnuthMorrisPratt(std::string& input, const std::unordered_map<std::string, std::string>& tokens)
{
    input.reserve(input.size() * 2);
    for(const auto& token : tokens)
    {
        auto next = boost::algorithm::knuth_morris_pratt_search(input.cbegin(), input.cend(), token.first.begin(), token.first.end());
        while(next.second != input.cend())
        {
            input.replace(next.first, next.second, token.second);
            next = boost::algorithm::boyer_moore_search(input.cbegin(), input.cend(), token.first.begin(), token.first.end());
        }
    }
}