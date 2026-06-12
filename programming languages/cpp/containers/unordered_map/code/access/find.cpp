#include <iostream>
#include <unordered_map>

template <typename K, typename V>
void _test_find(const std::unordered_map<K, V>&, const std::string&);

int main() {
    std::unordered_map<std::string, int> uomap {
        {"one", 1}
    };

    _test_find(uomap, "one");
    _test_find(uomap, "two");

    return 0;
}

template <typename K, typename V>
void _test_find(const std::unordered_map<K, V>& uomap, const std::string& key) {
    if (auto it = uomap.find(key); it != uomap.end()) {
        std::cout << key << " <= " << it->second << std::endl;
    } else {
        std::cout << key << " <= " << " does not exist" << std::endl;
    }
}
