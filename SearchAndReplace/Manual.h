#pragma once
#include <unordered_map>

namespace mfc
{
std::string::size_type Find(const std::string& input, const std::string& subString, std::string::size_type start);
}

void expandTokensNaiveSTL(std::string& input, const std::unordered_map<std::string, std::string>& tokens);
std::string expandTokensMFCMimicking(const std::string& input, const std::unordered_map<std::string, std::string>& tokens);
std::string expandTokensSehe(const std::string& input, const std::unordered_map<std::string, std::string>& tokens);
void expandTokensKreuzerkrieg(std::string& input, const std::unordered_map<std::string, std::string>& tokens);
