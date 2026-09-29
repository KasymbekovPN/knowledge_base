#include <string>
#include <deque>
#include <list>
#include <stack>
#include <optional>
#include <map>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <utility>

namespace {
    template<typename T>
    class LRUCache {
        std::mutex mutex_;
        std::list<std::pair<std::string, T>> order_;
        std::unordered_map<std::string, typename std::list<std::pair<std::string, T>>::iterator> cache_;
        size_t capacity_;

        void touch(const std::string& key) {
            order_.splice(order_.begin(), order_, cache_[key]);
        }

    public:

        explicit LRUCache(const size_t capacity): capacity_(capacity > 0 ? capacity : 100) {}

        void put(const std::string& key, T value) {
            std::lock_guard<std::mutex> lock(mutex_);

            if (cache_.contains(key)) {
                touch(key);
                cache_[key]->second = std::move(value);
            } else {
                order_.emplace_front(key, std::move(value));
                cache_[key] = order_.begin();
                if (order_.size() > capacity_) {
                    const auto fst{order_.back().first};
                    order_.pop_back();
                    cache_.erase(fst);
                }
            }
        }

        [[nodiscard]] std::optional<T> get(const std::string& key) {
            std::lock_guard<std::mutex> lock(mutex_);

            if (cache_.contains(key)) {
                touch(key);
                return cache_[key]->second;
            }

            return std::nullopt;
        }
    };
}

int main(int argc, char *argv[]) {
    return 0;
}
