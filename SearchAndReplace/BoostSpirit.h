#pragma once

#include <boost/spirit/include/qi.hpp>

namespace bsq = boost::spirit::qi;

std::string expandTokensBoostSpirit(const std::string& input, bsq::symbols<char, std::string> const& symbols);
