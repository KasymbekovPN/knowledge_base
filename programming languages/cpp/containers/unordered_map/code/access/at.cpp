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
    try {
        std::cout << "<= " << uomap.at(key) << std::endl;
    } catch(const std::out_of_range& e) {
        std::cerr << e.what() << std::endl;
    }
}
