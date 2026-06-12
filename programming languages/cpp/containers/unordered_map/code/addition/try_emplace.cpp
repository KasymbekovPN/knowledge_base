#include <iostream>
#include <unordered_map>

template <typename K, typename V>
void _print_pair(const std::pair<typename std::unordered_map<K, V>::iterator, bool>&);

template <typename K, typename V>
void _print_map(const std::unordered_map<K, V>&);

int main() {
    std::unordered_map<std::string, int> map;
    _print_pair<std::string, int>(map.try_emplace("one", 1));
    _print_pair<std::string, int>(map.try_emplace("one", 11));
    _print_pair<std::string, int>(map.try_emplace("one", 11));
    _print_pair<std::string, int>(map.try_emplace("two", 2));

    _print_map(map);

    return 0;
}

template <typename K, typename V>
void _print_pair(const std::pair<typename std::unordered_map<K, V>::iterator, bool>& pair) {
    std::cout
     << "[ (" << pair.first->first
     << ", " << pair.first->second
     << " ), " << pair.second
     << " ]" << std::endl;
}

template <typename K, typename V>
void _print_map(const std::unordered_map<K, V>& uomap) {
    for (auto &[key, value]: uomap) {
        std::cout
            << "{ " << key
            << ", " << value
            << " }" << std::endl;
    }
}
