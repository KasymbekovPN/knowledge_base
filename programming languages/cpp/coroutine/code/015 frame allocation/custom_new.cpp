#include <iostream>
#include <format>
#include <new>
#include <utility>
#include <coroutine>

static long g_heap_allocs{};
void* operator new(std::size_t _n) {
    g_heap_allocs++;
    if (void* p = std::malloc(_n)) return p;

    throw std::bad_alloc{};
}
void operator delete(void* _p) noexcept { std::free(_p); }
void operator delete(void* _p, std::size_t) noexcept { std::free(_p); }

// Простейшая арена: выдаёт куски из заранее выделенного буфера, не трогая кучу
struct Arena {
    static constexpr std::size_t SIZE = 64 * 1024;
    alignas(std::max_align_t) unsigned char buffer[SIZE];
    std::size_t offset{};
    long frame_allocs{};

    void* allocate(std::size_t _n) {
        _n = (_n + 15) & ~std::size_t(15); // выравнивание до 16
        if (offset + _n > SIZE) throw std::bad_alloc{};
        void* p = buffer + offset;
        offset += _n;
        frame_allocs++;

        return p;
    }
    void reset() { offset = 0; }
};

Arena g_arena;

struct Task {
    struct promise_type {
        // КАСТОМНЫЙ operator new: frame берётся из арены, НЕ из кучи
        static void* operator new(std::size_t _n) {
            return g_arena.allocate(_n);
        }
        // парный delete: арена освобождает оптом через reset(), тут no-op
        static void operator delete(void*, std::size_t) noexcept {}

        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {};}
        void return_void() {}
        void unhandled_exception() {}
    };

    std::coroutine_handle<promise_type> h;

    explicit Task(std::coroutine_handle<promise_type> _h): h{_h} {}
    Task(Task&& _other) noexcept: h(std::exchange(_other.h, {})) {}
    ~Task() { if (h) h.destroy(); }
};

Task worker(int _id) {
    co_await std::suspend_always();
    std::cout << std::format("[worker] {}\n", _id);

    co_return;
}

int main() {
    std::cout << std::format("start: heap_allocs: {}", g_heap_allocs);

    // создаём МНОГО корутин — все frame'ы идут в арену, куча не трогается
    constexpr int N{1000};

    // 1 аллокация под массив
    Task* tasks = static_cast<Task*>(::operator new(sizeof(Task) * N));
    long heap_before{g_heap_allocs};

    for (int i{}; i < N; ++i) {
        new (&tasks[i]) Task(worker(i)); // создание корутины -> frame в арене
    }

    std::cout << std::format("after creating {} coroutines\n", N);
    std::cout << std::format("  frames in area = {}\n", g_arena.frame_allocs);
    std::cout << std::format(
        "  g_heap_allocs for frame = {} (must be 0)\n",
        g_heap_allocs - heap_before
    );

    for (int i{}; i < N; ++i) {
        auto&& task = tasks[i];
        task.h.resume();
        task.h.resume();
        task.~Task();
    }

    ::operator delete(tasks);

    return 0;
}
