
// #include <atomic>
// #include <memory>
//
// template<typename T>
// class TreiberStack {
//     struct Node {
//         T data;
//         Node* next;
//         Node(T val) : data(std::move(val)), next(nullptr) {}
//     };
//
//     std::atomic<Node*> head{nullptr};
//
// public:
//     void push(T value) {
//         Node* new_node = new Node(std::move(value));
//
//         // (1) relaxed: пока узел никому не виден, порядок не важен
//         new_node->next = head.load(std::memory_order_relaxed);
//
//         // (2) CAS: acq_rel
//         //   - acquire-часть: если CAS не удался, нам нужно увидеть
//         //     актуальный head для следующей попытки
//         //   - release-часть: если CAS удался, публикуем new_node —
//         //     всё, что записано в него (data, next) в шаге (1),
//         //     станет видно потоку, который потом сделает pop()
//         while (!head.compare_exchange_weak(
//                    new_node->next,      // expected (обновляется при неудаче)
//                    new_node,            // desired
//                    std::memory_order_acq_rel,   // при успехе
//                    std::memory_order_relaxed))  // при неудаче достаточно relaxed
//         {
//             // new_node->next уже обновлён compare_exchange_weak до
//             // актуального head — просто повторяем попытку
//         }
//     }
//
//     bool pop(T& result) {
//         Node* old_head = head.load(std::memory_order_acquire);
//
//         while (old_head != nullptr &&
//                !head.compare_exchange_weak(
//                    old_head,
//                    old_head->next,
//                    std::memory_order_acq_rel,   // успех: читаем + публикуем
//                    std::memory_order_acquire))  // неудача: нужен свежий head
//         {
//             // old_head обновлён автоматически, повторяем
//         }
//
//         if (old_head == nullptr) return false;
//
//         // (3) Здесь безопасно читать old_head->data — happens-before
//         // от push() гарантирует, что данные видны
//         result = std::move(old_head->data);
//         delete old_head; // УПРОЩЕНИЕ: в реальности тут ABA problem +
//                           // use-after-free нужно решать через hazard
//                           // pointers или epoch-based reclamation
//         return true;
//     }
// };

int main(int argc, char *argv[]) {
    return 0;
}
