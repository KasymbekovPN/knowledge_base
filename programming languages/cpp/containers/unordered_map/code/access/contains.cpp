#include <iostream>
#include <unordered_map>

template <typename K, typename V>
void _test_contains(const std::unordered_map<K, V>&, const std::string&);

int main() {
    std::unordered_map<std::string, int> uomap {
        {"one", 1}
    };

    _test_contains(uomap, "one");
    _test_contains(uomap, "two");

    return 0;
}

template <typename K, typename V>
void _test_contains(const std::unordered_map<K, V>& uomap, const std::string& key) {
    if (uomap.contains(key)) {
        std::cout << key << " <= " << uomap[key] << std::endl;
    } else {
        std::cout << key << " <= " << " does not exist" << std::endl;
    }
}
