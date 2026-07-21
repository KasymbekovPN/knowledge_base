#pragma once

#include <list>
#include <unordered_map>
#include <utility>

namespace lru_cache {

    class LRUCache {
    public:
        explicit LRUCache(const int capacity): capacity_(capacity) {}
        int get(const int);
        void put(const int, const int);
    private:
        using ListIt = std::list<std::pair<int, int>>::iterator;

        void touch(ListIt);

        int capacity_;
        std::list<std::pair<int, int>> order_;
        std::unordered_map<int, ListIt> index_;
    };

    void demo();

}