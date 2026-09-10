#include <iostream>
#include <format>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

namespace {
    class LRUCache {
        struct Node {
            int key;
            int value;
            Node* next;
            Node* prev;

            explicit Node(const int key, const int value):
                key{key},
                value{value},
                next{nullptr},
                prev{nullptr} {}
        };

        static constexpr int DEFAULT_SIZE{10};
        static constexpr int ERR_ABSENT{-1};

        int capacity{0};
        std::unordered_map<int, Node*> cache;
        Node* head{nullptr}; // dummy, сразу за ним — самый свежий
        Node* tail{nullptr}; // dummy, перед ним — самый старый (кандидат на вытеснение)

        std::shared_mutex cache_mutex;

        static void remove(const Node* node) {
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }

        void insert_front(Node* node) const {
            node->next = head->next;
            node->prev = head;
            head->next->prev = node;
            head->next = node;
        }

    public:
        explicit LRUCache(const int capacity)
            : capacity(capacity <= 0 ? DEFAULT_SIZE : capacity) {
            head = new Node{0, 0};
            tail = new Node{0, 0};
            head->next = tail;
            tail->prev = head;
        }

        int get(const int key) {
            std::shared_lock<std::shared_mutex> lock{cache_mutex};

            if (!cache.contains(key))
                return ERR_ABSENT;
            Node* node{cache[key]};
            remove(node);
            insert_front(node);

            return node->value;
        }

        void put(const int key, const int value) {
            std::unique_lock<std::shared_mutex> lock{cache_mutex};

            if (cache.contains(key)) {
                remove(cache[key]);
                delete cache[key];
            }

            const auto node{new Node{key, value}};
            cache[key] = node;
            insert_front(node);

            if (cache.size() <= capacity) return;

            const auto lru = tail->prev;
            remove(lru);
            cache.erase(lru->key);
            delete lru;
        }
    };
}

int main() {
    LRUCache cache(2);

    cache.put(1, 10);
    cache.put(2, 20);
    std::cout << "get(1): " << cache.get(1) << std::endl; // 10, элемент 1 стал "свежим"

    cache.put(3, 30); // capacity=2 превышена -> вытесняется наименее недавно использованный (ключ 2)
    std::cout << "get(2): " << cache.get(2) << std::endl; // -1, потому что был вытеснен

    cache.put(4, 40); // снова превышение -> вытесняется ключ 1 (он не трогался с момента get(1))
    std::cout << "get(1): " << cache.get(1) << std::endl; // -1
    std::cout << "get(3): " << cache.get(3) << std::endl; // 30
    std::cout << "get(4): " << cache.get(4) << std::endl; // 40

    return 0;
}
