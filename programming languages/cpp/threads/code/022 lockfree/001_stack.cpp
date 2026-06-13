#include <iostream>
#include <memory>
#include <atomic>
#include <optional>
#include <thread>

template<typename T>
class Stack {
    struct Node {
        T value;
        Node* next;
        Node(T v): value{std::move(v)}, next{nullptr} {}
    };

public:
    void push(T _value) {
        Node* new_node = new Node{std::move(_value)};
        new_node->next = header.load();
        // CAS: in case of header did not changed then write new_node
        while (!header.compare_exchange_weak(new_node->next, new_node));
    }

    std::optional<T> pop() {
        Node* old_header = header.load();
        while (
            old_header &&
            !header.compare_exchange_weak(old_header, old_header->next)
        );
        if (!old_header) return std::nullopt;

        T value = std::move(old_header->value);
        delete old_header;

        return value;
    }

private:
    std::atomic<Node*> header{nullptr};
};

int main() {
    Stack<int> stack;

    std::jthread writer0{[&]() {
        for (int i{}; i < 3; i++) {
            stack.push(i);
        }
    }};
    std::jthread writer1{[&]() {
        for (int i{}; i < 3; i++) {
            stack.push(i);
        }
    }};
    std::jthread reader{[&](std::stop_token _stoken) {
        while (!_stoken.stop_requested()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            auto&& taken = stack.pop();
            if (!taken) {
                continue;
            }

            std::cout << "reader: " << taken.value() << "\n";
        }
    }};

    std::this_thread::sleep_for(std::chrono::seconds(1));
    reader.request_stop();

    return 0;
}
