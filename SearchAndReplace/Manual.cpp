#include "Manual.h"
#include <algorithm>
#include <cstring>
#include <sstream>

void expandTokensNaiveSTL(std::string& input, const std::unordered_map<std::string, std::string>& tokens)
{
    input.reserve(input.size() * 2);
    for(const auto& token : tokens)
    {
        auto next = std::search(input.cbegin(), input.cend(), token.first.begin(), token.first.end());
        while(next != input.cend())
        {
            input.replace(next, next + token.first.size(), token.second);
            next = std::search(input.cbegin(), input.cend(), token.first.begin(), token.first.end());
        }
    }
}

namespace mfc
{
std::string::size_type Find(const std::string& input, const std::string& subString, std::string::size_type start)
{
    if(subString.empty())
    {
        return std::string::npos;
    }

    if(start < 0 || start > input.size())
    {
        return std::string::npos;
    }

    auto found = strstr(input.c_str() + start, subString.c_str());
    return ((found == nullptr) ? std::string::npos : std::string::size_type(found - input.c_str()));
}
} // namespace mfc

std::string expandTokensMFCMimicking(const std::string& input, const std::unordered_map<std::string, std::string>& tokens)
{
    auto tmpInput = input;
    for(const auto& token : tokens)
    {
        auto pos = 0;
        while(pos != std::string::npos)
        {
            pos = mfc::Find(tmpInput, token.first, pos);
            if(pos != std::string::npos)
            {
                auto tokenLength = token.first.size();
                tmpInput.replace(pos, tokenLength, token.second.c_str());
                pos += 1;
            }
        }
    }

    return tmpInput;
}
std::string expandTokensSehe(const std::string& input, const std::unordered_map<std::string, std::string>& tokens)
{
    std::ostringstream builder;
    auto expand = [&](auto const& key) {
        auto match = tokens.find(key);
        if(match == tokens.end())
            builder << "$(" << key << ")";
        else
            builder << match->second;
    };

    builder.str().reserve(input.size() * 2);

    builder.str("");
    std::ostreambuf_iterator<char> out(builder);

    for(auto f(input.begin()), l(input.end()); f != l;)
    {
        switch(*f)
        {
        case '$':
        {
            if(++f == l || *f != '(')
            {
                *out++ = '$';
                break;
            }
            else
            {
                auto s = ++f;
                size_t n = 0;

                while(f != l && *f != ')')
                    ++f, ++n;

                // key is [s,f] now
                expand(std::string(&*s, &*s + n));

                if(f != l)
                    ++f; // skip '}'
            }
        }
        default:
            *out++ = *f++;
        }
    }
    return builder.str();
}

void expandTokensKreuzerkrieg(std::string& input, const std::unordered_map<std::string, std::string>& tokens)
{
    std::string builder;
    builder.reserve(input.size() * 2);
    auto expand = [&builder, &tokens](auto const& key) {
        auto match = tokens.find(key);

        if(match == tokens.end())
        {
            return false;
        }
        else
        {
            builder.append(match->second);
            return true;
        }
    };

    auto begin = input.c_str();
    auto end = begin + input.size();
    while(begin < end)
    {
        auto span = strchr(begin, '$');
        if(span != nullptr)
        {
            builder.append(begin, span);
            begin = span;
            auto tokenStart = begin;
            if(begin < end)
            {
                if(*(++begin) == '(')
                {
                    ++begin;
                    auto span = strchr(begin, ')');
                    if(span > nullptr)
                    {
                        std::string token(begin, span);
                        if(!expand(token))
                        {
                            builder.append(tokenStart, span + 3);
                        }
                        begin = span;
                        ++begin;
                    }
                    else
                    {
                        builder += "$(";
                    }
                }
                else
                {
                    builder += '$';
                }
            }
        }
        else
        {
            builder.append(begin, end);
            begin = end;
        }
    }
    input = builder;
}