#include "lru_cache.h"

#include <iostream>
#include <format>

namespace lru_cache {

int LRUCache::get(int key) {
    auto it{index_.find(key)};
    if (it == index_.end()) {
        return -1;
    }
    touch(it->second);

    return it->second->second;
}

void LRUCache::put(int key, int value) {
    auto it{index_.find(key)};

    if (it != index_.end()) {
        it->second->second = value;
        touch(it->second);
        return;
    }

    if (static_cast<int>(order_.size()) >= capacity_) {
        // вытесняем наименее недавно использованный (хвост списка)
        auto& lru = order_.back();
        index_.erase(lru.first);
        order_.pop_back();
    }

    order_.emplace_front(key, value);
    index_[key] = order_.begin();
}

void LRUCache::touch(ListIt it) {
    order_.splice(order_.begin(), order_, it);
}

void demo() {
    auto cache = LRUCache(3);
    cache.put(0, 100);
    cache.put(1, 101);
    cache.put(2, 102);
    cache.put(3, 103);

    cache.get(3);
    cache.put(4, 104);

    for (int i{0}; i < 5; ++i) {
        std::cout << std::format("{} <-> {}\n", i, cache.get(i));
    }
}

}
