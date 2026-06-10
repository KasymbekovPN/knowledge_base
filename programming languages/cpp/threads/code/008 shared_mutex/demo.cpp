#include <iostream>
#include <thread>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

struct Cache {
public:
    std::string get(const std::string& _key) {
        std::shared_lock lock{mtx};
        auto&& it = data.find(_key);
        return it != data.end() ? it->second : "";
    }

    void set(const std::string &_key,
             const std::string &_value) {
        std::unique_lock lock{mtx};
        data[_key] = _value;
    }

private:
    std::shared_mutex mtx;
    std::unordered_map<std::string, std::string> data;
};

int main(int argc, char const *argv[]) {
    Cache cache;
    cache.set("user", "Alice");

    std::thread writer{[&]() { cache.set("user", "Bob"); }};
    writer.join();

    std::vector<std::thread> readers;
    for (int i = 0; i < 30; i++) {
        readers.emplace_back([&]() {
            std::cout << cache.get("user") << std::endl;
        });
    }

    for (auto& t : readers) {
        t.join();
    }

    return 0;
}
