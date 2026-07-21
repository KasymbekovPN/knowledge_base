#include "group_anagram.h"

#include <iostream>
#include <format>
#include <algorithm>
#include <ranges>
#include <unordered_map>

namespace group_anagram {

std::vector<std::vector<std::string>> group_anagrams(std::vector<std::string> &input) {
    std::unordered_map<std::string, std::vector<std::string>> groups;
    groups.reserve(input.size());

    for (auto& s: input) {
        std::string key{s};
        std::ranges::sort(key.begin(), key.end());
        groups[key].push_back(std::move(s));
    }

    std::vector<std::vector<std::string>> result;
    result.reserve(groups.size());
    for (auto &group: groups | std::views::values) {
        result.push_back(std::move(group));
    }

    return result;
}

void demo() {
    std::vector<std::string> LINES{"eat", "tea", "tan", "ate", "nat", "bat"};
    for (const auto& vectors = group_anagrams(LINES); auto& vector : vectors) {
        std::string delimiter;
        std::cout << "{";
        for (auto& item : vector) {
            std::cout << std::format("{}{}", delimiter, item);
            delimiter = ", ";
        }
        std::cout << "}\n";
    }
}

}