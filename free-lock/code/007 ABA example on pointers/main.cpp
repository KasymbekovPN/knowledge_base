#include <atomic>
#include <print>
#include <array>
#include <cstdint>
#include <iostream>
#include <string>

// ============================================================
// ЧАСТЬ 1: Raw pointer -- ABA приводит к segfault
// ============================================================
namespace raw_demo {
    namespace {
        struct Node {
            int value;
            Node* next;
        };
        std::atomic<Node*> head{nullptr};

        void print_stack(const std::string& label) {
            std::print(std::cout, "{}: ", label);
            for (const Node* n{head.load()}; n; n = n->next) {
                std::cout << n->value << "(@" << n << ") -> ";
            }
            std::cout << "nullptr\n" << std::flush;
        }

        void run() {
            std::cout << "\n========== 1. RAW POINTER (ABA -> segfault) ==========\n" << std::flush;

            Node* A{new Node{.value = 1, .next = nullptr}};
            Node* B{new Node{.value = 2, .next = nullptr}};
            Node* C{new Node{.value = 3, .next = nullptr}};
            A->next = B; B->next = C;
            head.store(A);
            print_stack("Start");

            Node* old_head{head.load()};
            Node* observed_next{old_head->next}; // B, пока A жив
            std::cout << "[thread 1] read A, next=B. STOPPED" << std::flush;

            Node* p1{head.load()}; head.store(p1->next); delete p1; // снимает A
            Node* p2{head.load()}; head.store(p2->next); delete p2; // снимает B
            Node* D{new (A) Node{.value = 42, .next = nullptr}}; // D переиспользует адрес A!
            D->next = head.load(); head.store(D);
            print_stack("After pop/pop/push(D) by thread 2");
            std::cout << "Address D == Address A? " << (D == A ? "yes" : "no") << "\n" << std::flush;

            Node* expected{old_head};
            bool success{head.compare_exchange_strong(expected, observed_next)};
            std::cout << "[Thread 1] CAS: " << (success ? "Success (bag!)" : "fail") << "\n" << std::flush;

            std::cout << "The next access to the stack dereferences the removed B -> segfault.:\n" << std::flush;
            print_stack("BOTTOM LINE (we expect a collapse)");
        }

    }
}

// ============================================================
// ЧАСТЬ 2: Tagged pointer -- CAS корректно проваливается
// ============================================================
namespace target_demo {
    // struct Node { int value; Node* next; };
// struct TaggedPtr {
//     Node* ptr; uint64_t tag;
//     bool operator==(const TaggedPtr& o) const { return ptr == o.ptr && tag == o.tag; }
// };
// std::atomic<TaggedPtr> head{TaggedPtr{nullptr, 0}};
//
// void print_stack(const char* label) {
//     TaggedPtr h = head.load();
//     std::cout << label << ": [tag=" << h.tag << "] ";
//     for (Node* n = h.ptr; n; n = n->next)
//         std::cout << n->value << "(@" << n << ") -> ";
//     std::cout << "nullptr\n" << std::flush;
// }
//
// void run() {
//     std::cout << "\n========== 2. TAGGED POINTER (CAS проваливается, стек цел) ==========\n" << std::flush;
//
//     Node* A = new Node{1, nullptr};
//     Node* B = new Node{2, nullptr};
//     Node* C = new Node{3, nullptr};
//     A->next = B; B->next = C;
//     head.store(TaggedPtr{A, 0});
//     print_stack("Начало");
//
//     TaggedPtr old_head = head.load();
//     Node* observed_next = old_head.ptr->next;
//     std::cout << "[Поток 1] прочитал {A, tag=0}. ПРИОСТАНОВЛЕН.\n" << std::flush;
//
//     TaggedPtr h1 = head.load(); Node* p1 = h1.ptr;
//     head.store(TaggedPtr{p1->next, h1.tag + 1}); delete p1;
//
//     TaggedPtr h2 = head.load(); Node* p2 = h2.ptr;
//     head.store(TaggedPtr{p2->next, h2.tag + 1}); delete p2;
//
//     Node* D = new (A) Node{99, nullptr};
//     TaggedPtr h3 = head.load();
//     D->next = h3.ptr;
//     head.store(TaggedPtr{D, h3.tag + 1});
//     print_stack("После pop/pop/push(D) потоком 2");
//     std::cout << "tag сейчас = " << head.load().tag << " (был 0 при чтении потоком 1)\n" << std::flush;
//
//     TaggedPtr expected = old_head;
//     TaggedPtr desired = TaggedPtr{observed_next, old_head.tag + 1};
//     bool success = head.compare_exchange_strong(expected, desired);
//     std::cout << "[Поток 1] CAS: " << (success ? "успех (БАГ!)" : "НЕУДАЧА (правильно)")
//               << " -- указатель совпал, но tag " << expected.tag << " != " << old_head.tag << "\n" << std::flush;
//
//     print_stack("ИТОГ (стек не повреждён)");
// }
}

// ============================================================
// ЧАСТЬ 3: Hazard pointer -- память не освобождается, пока защищена
// ============================================================
namespace hazard_demo {
// struct Node { int value; Node* next; };
// std::atomic<Node*> head{nullptr};
// constexpr int MAX_HAZARDS = 4;
// std::array<std::atomic<Node*>, MAX_HAZARDS> hazard_ptrs;
//
// bool is_hazardous(Node* p) {
//     for (auto& h : hazard_ptrs)
//         if (h.load(std::memory_order_acquire) == p) return true;
//     return false;
// }
//
// void retire(Node* p) {
//     if (is_hazardous(p)) {
//         std::cout << "  [retire] " << p << " защищён -- delete отложен\n" << std::flush;
//     } else {
//         std::cout << "  [retire] " << p << " не защищён -- удаляем сразу\n" << std::flush;
//         delete p;
//     }
// }
//
// void print_stack(const char* label) {
//     std::cout << label << ": ";
//     for (Node* n = head.load(); n; n = n->next)
//         std::cout << n->value << "(@" << n << ") -> ";
//     std::cout << "nullptr\n" << std::flush;
// }
//
// void run() {
//     std::cout << "\n========== 3. HAZARD POINTER (память жива, пока защищена) ==========\n" << std::flush;
//     for (auto& h : hazard_ptrs) h.store(nullptr);
//
//     Node* A = new Node{1, nullptr};
//     Node* B = new Node{2, nullptr};
//     Node* C = new Node{3, nullptr};
//     A->next = B; B->next = C;
//     head.store(A);
//     print_stack("Начало");
//
//     Node* old_head = head.load(std::memory_order_acquire);
//     hazard_ptrs[0].store(old_head, std::memory_order_release); // защищаем A
//     Node* observed_next = old_head->next;
//     std::cout << "[Поток 1] hazard = A. ПРИОСТАНОВЛЕН.\n" << std::flush;
//
//     Node* p1 = head.load(); head.store(p1->next); retire(p1); // A под защитой -- не удалится
//     Node* p2 = head.load(); head.store(p2->next); retire(p2); // B удалится сразу
//
//     Node* D = new Node{99, nullptr}; // обычный new -- адрес A занят, аллокатор его не тронет
//     D->next = head.load(); head.store(D);
//     print_stack("После pop/pop/push(D) потоком 2");
//     std::cout << "Адрес D == адресу A? " << (D == A ? "да" : "НЕТ (A ещё жив)") << "\n" << std::flush;
//
//     Node* expected = old_head;
//     bool success = head.compare_exchange_strong(expected, observed_next);
//     std::cout << "[Поток 1] CAS: " << (success ? "успех" : "неудача (ожидаемо)") << "\n" << std::flush;
//
//     hazard_ptrs[0].store(nullptr, std::memory_order_release);
//     if (!is_hazardous(old_head)) {
//         std::cout << "  [reclaim] A больше не защищён -- удаляем\n" << std::flush;
//         delete old_head;
//     }
//
//     print_stack("ИТОГ (стек цел, без segfault)");
// }
}

int main(const int argc, char *argv[]) {
    if (const bool run_raw{argc > 1 && std::string(argv[1]) == "--raw"};
        run_raw) {
        raw_demo::run();
    } else {
        //         tagged_demo::run();
        //         hazard_demo::run();
        //         std::cout << "\n(Запустите с флагом --raw, чтобы увидеть падение "
        //                   << "raw-pointer версии: она вызывает segfault намеренно)\n" << std::flush;
    }

    return 0;
}
