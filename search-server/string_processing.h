#pragma once

#include <set>
#include <string>
#include <vector>

template <typename StringContainer>
std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer& strings);

std::vector<std::string> SplitIntoWords(const std::string& text);