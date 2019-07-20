#pragma once

#include <unordered_map>

std::string expandTokensBoostReplaceAll(const std::string& input, const std::unordered_map<std::string, std::string>& tokens);
void expandTokensBoostBoyerMoore(std::string& input, const std::unordered_map<std::string, std::string>& tokens);
void expandTokensBoyerMooreHorspool(std::string& input, const std::unordered_map<std::string, std::string>& tokens);
void expandTokensBoostKnuthMorrisPratt(std::string& input, const std::unordered_map<std::string, std::string>& tokens);
